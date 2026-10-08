#include "scheduler.h"

#include "dataset.h"
#include "omp_engine.h"

#include <mpi.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

namespace
{
    constexpr int TAG_WORK = 100;
    constexpr int TAG_RESULT = 101;
    constexpr int TAG_STOP = 102;

    struct ResultPacket
    {
        long long records;
        long long candidates;
        long long normal;
        long long low;
        long long medium;
        long long high;

        // Load-balance measurements
        int chunks_processed;
        double processing_time;
    };

    struct RankStats
    {
        long long records = 0;
        int chunks = 0;
        double processing_time = 0.0;
    };
}

AnalysisStats run_adaptive(
    const std::string& input,
    int rank,
    int world_size,
    int chunks_per_worker)
{
    /*
     * Rank 0 = scheduler.
     * Other ranks = workers.
     */

    if (world_size < 2)
    {
        return {};
    }

    long long file_size =
        dataset_size(input);

    int worker_count =
        world_size - 1;

    int total_chunks =
        worker_count *
        chunks_per_worker;

    if (total_chunks < 1)
    {
        total_chunks = 1;
    }

    long long chunk_size =
        std::max(
            1LL,
            file_size / total_chunks
        );

    /*
     * SCHEDULER
     */
    if (rank == 0)
    {
        std::vector<FileRange>
            work_queue;

        for (int i = 0;
             i < total_chunks;
             ++i)
        {
            long long begin =
                static_cast<long long>(i)
                * chunk_size;

            long long end =
                (i == total_chunks - 1)
                ? file_size
                : std::min(
                    file_size,
                    begin + chunk_size
                );

            work_queue.push_back(
                {
                    begin,
                    end
                }
            );
        }

        AnalysisStats global;

        int next_chunk = 0;
        int completed_chunks = 0;

        /*
         * Store load information for
         * every MPI worker.
         */
        std::vector<RankStats>
            rank_stats(world_size);

        /*
         * Give initial work.
         */
        for (
            int worker = 1;
            worker < world_size &&
            next_chunk < total_chunks;
            ++worker
        )
        {
            long long message[2] =
            {
                work_queue[next_chunk].begin,
                work_queue[next_chunk].end
            };

            MPI_Send(
                message,
                2,
                MPI_LONG_LONG,
                worker,
                TAG_WORK,
                MPI_COMM_WORLD
            );

            next_chunk++;
        }

        /*
         * Dynamic work distribution.
         */
        while (
            completed_chunks <
            total_chunks
        )
        {
            MPI_Status status;

            ResultPacket result{};

            MPI_Recv(
                &result,
                sizeof(ResultPacket),
                MPI_BYTE,
                MPI_ANY_SOURCE,
                TAG_RESULT,
                MPI_COMM_WORLD,
                &status
            );

            AnalysisStats local;

            local.records =
                result.records;

            local.candidates =
                result.candidates;

            local.normal =
                result.normal;

            local.low =
                result.low;

            local.medium =
                result.medium;

            local.high =
                result.high;

            merge_stats(
                global,
                local
            );

            completed_chunks++;

            int worker =
                status.MPI_SOURCE;

            /*
             * Accumulate worker load
             * information.
             */
            rank_stats[worker].records +=
                result.records;

            rank_stats[worker].chunks +=
                result.chunks_processed;

            rank_stats[worker].processing_time +=
                result.processing_time;

            /*
             * Worker finished.
             * Give it more work.
             */
            if (
                next_chunk <
                total_chunks
            )
            {
                long long message[2] =
                {
                    work_queue[next_chunk].begin,
                    work_queue[next_chunk].end
                };

                MPI_Send(
                    message,
                    2,
                    MPI_LONG_LONG,
                    worker,
                    TAG_WORK,
                    MPI_COMM_WORLD
                );

                next_chunk++;
            }
            else
            {
                /*
                 * No more work.
                 */
                MPI_Send(
                    nullptr,
                    0,
                    MPI_BYTE,
                    worker,
                    TAG_STOP,
                    MPI_COMM_WORLD
                );
            }
        }

        /*
         * Calculate load-balance metrics.
         */
        double minimum_time =
            0.0;

        double maximum_time =
            0.0;

        double total_time =
            0.0;

        int active_workers =
            0;

        for (
            int worker = 1;
            worker < world_size;
            ++worker
        )
        {
            double worker_time =
                rank_stats[worker]
                    .processing_time;

            if (rank_stats[worker].chunks > 0)
            {
                if (active_workers == 0)
                {
                    minimum_time =
                        worker_time;

                    maximum_time =
                        worker_time;
                }
                else
                {
                    minimum_time =
                        std::min(
                            minimum_time,
                            worker_time
                        );

                    maximum_time =
                        std::max(
                            maximum_time,
                            worker_time
                        );
                }

                total_time +=
                    worker_time;

                active_workers++;
            }
        }

        double average_time =
            active_workers > 0
            ? total_time /
              active_workers
            : 0.0;

        /*
         * Load imbalance is measured
         * relative to the average worker
         * processing time.
         */
        double load_imbalance =
            average_time > 0.0
            ? (
                (maximum_time -
                 minimum_time)
                / average_time
              ) * 100.0
            : 0.0;

        std::cout
            << "\n============================================\n";

        std::cout
            << "       CYBERSENTINEL LOAD BALANCE\n";

        std::cout
            << "============================================\n\n";

        std::cout
            << "Adaptive scheduler statistics\n";
        
        std::cout
            << "MPI processes        : "
            << world_size
            << "\n";

        std::cout
            << "Worker ranks         : "
            << worker_count
            << "\n";

        std::cout
            << "Total chunks         : "
            << total_chunks
            << "\n";

        std::cout
            << "Chunks per worker    : "
            << chunks_per_worker
            << "\n\n";

        std::cout
            << "--------------------------------------------\n";

        std::cout
            << "Per-rank workload\n";

        std::cout
            << "--------------------------------------------\n";

        std::cout
            << std::fixed
            << std::setprecision(4);

        for (
            int worker = 1;
            worker < world_size;
            ++worker
        )
        {
            std::cout
                << "Rank "
                << worker
                << " | Chunks: "
                << rank_stats[worker].chunks
                << " | Records: "
                << rank_stats[worker].records
                << " | Time: "
                << rank_stats[worker].processing_time
                << " s\n";
        }

        std::cout
            << "\n--------------------------------------------\n";

        std::cout
            << "Load-balance metrics\n";

        std::cout
            << "--------------------------------------------\n";

        std::cout
            << "Maximum rank time : "
            << maximum_time
            << " s\n";

        std::cout
            << "Minimum rank time : "
            << minimum_time
            << " s\n";

        std::cout
            << "Average rank time : "
            << average_time
            << " s\n";

        std::cout
            << "Load imbalance    : "
            << load_imbalance
            << " %\n";

        std::cout
            << "--------------------------------------------\n";

        std::cout
            << "Adaptive scheduling completed successfully.\n";

        std::cout
            << "============================================\n\n";

        return global;
    }

    /*
     * WORKER
     */
    int local_chunks =
        0;

    long long local_records =
        0;

    double local_processing_time =
        0.0;

    while (true)
    {
        MPI_Status status;

        MPI_Probe(
            0,
            MPI_ANY_TAG,
            MPI_COMM_WORLD,
            &status
        );

        if (
            status.MPI_TAG ==
            TAG_STOP
        )
        {
            MPI_Recv(
                nullptr,
                0,
                MPI_BYTE,
                0,
                TAG_STOP,
                MPI_COMM_WORLD,
                &status
            );

            break;
        }

        long long message[2];

        MPI_Recv(
            message,
            2,
            MPI_LONG_LONG,
            0,
            TAG_WORK,
            MPI_COMM_WORLD,
            &status
        );

        FileRange range
        {
            message[0],
            message[1]
        };

        /*
         * Measure the processing time
         * for this individual chunk.
         */
        double start =
            MPI_Wtime();

        std::vector<NetworkRecord>
            records =
                load_range(
                    input,
                    range
                );

        /*
         * OpenMP works inside
         * each MPI worker.
         */
        AnalysisStats local =
            run_openmp(records);

        double end =
            MPI_Wtime();

        double elapsed =
            end - start;

        /*
         * Update local worker
         * load statistics.
         */
        local_chunks++;

        local_records +=
            local.records;

        local_processing_time +=
            elapsed;

        ResultPacket result
        {
            local.records,
            local.candidates,
            local.normal,
            local.low,
            local.medium,
            local.high,
            1,
            elapsed
        };

        MPI_Send(
            &result,
            sizeof(ResultPacket),
            MPI_BYTE,
            0,
            TAG_RESULT,
            MPI_COMM_WORLD
        );
    }

    return {};
}