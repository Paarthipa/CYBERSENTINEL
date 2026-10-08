#include "analyzer.h"
#include "dataset.h"
#include "mpi_engine.h"
#include "performance.h"
#include "scheduler.h"
#include "threat_detector.h"

#include <mpi.h>
#include <omp.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

struct Options
{
    std::string mode;
    std::string input;

    int chunks = 4;
};

static void print_usage()
{
    std::cout
        << "\nCYBERSENTINEL\n"
        << "Distributed Network Anomaly Detection\n\n"

        << "Usage:\n"
        << "./bin/cybersentinel "
        << "--mode MODE "
        << "--input FILE "
        << "[--chunks N]\n\n"

        << "Modes:\n"
        << "  seq       Sequential\n"
        << "  mpi       MPI\n"
        << "  hybrid    MPI + OpenMP\n"
        << "  adaptive  Adaptive Hybrid\n\n";
}

static Options parse_arguments(
    int argc,
    char* argv[])
{
    Options options;

    for (int i = 1;
         i < argc;
         ++i)
    {
        std::string argument =
            argv[i];

        if (
            argument == "--mode" &&
            i + 1 < argc
        )
        {
            options.mode =
                argv[++i];
        }
        else if (
            argument == "--input" &&
            i + 1 < argc
        )
        {
            options.input =
                argv[++i];
        }
        else if (
            argument == "--chunks" &&
            i + 1 < argc
        )
        {
            options.chunks =
                std::stoi(argv[++i]);
        }
    }

    return options;
}

static void print_statistics(
    const AnalysisStats& stats,
    double seconds)
{
    std::cout
        << "Records analysed   : "
        << stats.records
        << '\n';

    std::cout
        << "Suspicious flows   : "
        << stats.candidates
        << '\n';

    std::cout
        << "Normal             : "
        << stats.normal
        << '\n';

    std::cout
        << "Low                : "
        << stats.low
        << '\n';

    std::cout
        << "Medium             : "
        << stats.medium
        << '\n';

    std::cout
        << "High               : "
        << stats.high
        << '\n';

    std::cout
        << std::fixed
        << std::setprecision(4);

    std::cout
        << "Execution time (s) : "
        << seconds
        << '\n';

    std::cout
        << "Throughput (rec/s) : "
        << throughput(
            stats.records,
            seconds
        )
        << '\n';

    std::cout
        << "\nTOP THREAT SOURCES\n";

    std::cout
        << "--------------------------------------------\n";

    for (int i = 0;
         i < stats.top_count;
         ++i)
    {
        std::cout
            << std::setw(2)
            << i + 1
            << "  "

            << std::left
            << std::setw(16)

            << int_to_ip(
                stats.top[i].source_ip
            )

            << " score="
            << std::setw(7)
            << stats.top[i].score

            << " port="
            << stats.top[i].destination_port

            << '\n';
    }
}

int main(
    int argc,
    char* argv[])
{
    Options options =
        parse_arguments(
            argc,
            argv
        );

    if (
        options.mode.empty() ||
        options.input.empty()
    )
    {
        print_usage();
        return 1;
    }

    int provided = 0;

    MPI_Init_thread(
        &argc,
        &argv,
        MPI_THREAD_FUNNELED,
        &provided
    );

    int rank = 0;
    int world_size = 1;

    MPI_Comm_rank(
        MPI_COMM_WORLD,
        &rank
    );

    MPI_Comm_size(
        MPI_COMM_WORLD,
        &world_size
    );

    /*
     * ========================================
     * SEQUENTIAL
     * ========================================
     */
    if (options.mode == "seq")
    {
        if (rank == 0)
        {
            auto start =
                std::chrono::steady_clock::now();

            std::vector<NetworkRecord>
                records =
                    load_all(
                        options.input
                    );

            AnalysisStats stats =
                analyse_sequential(
                    records
                );

            auto end =
                std::chrono::steady_clock::now();

            double seconds =
                std::chrono::duration<double>(
                    end - start
                ).count();

            std::cout
                << "\n============================================\n"
                << "       CYBERSENTINEL - SEQUENTIAL\n"
                << "============================================\n"

                << "Dataset            : "
                << options.input
                << '\n'

                << "MPI processes      : 1\n"
                << "OpenMP threads     : 1\n"

                << "--------------------------------------------\n";

            print_statistics(
                stats,
                seconds
            );
        }

        MPI_Finalize();

        return 0;
    }

    /*
     * ========================================
     * MPI
     * ========================================
     */
    if (options.mode == "mpi")
    {
        MPI_Barrier(
            MPI_COMM_WORLD
        );

        auto start =
            std::chrono::steady_clock::now();

        AnalysisStats local =
            run_mpi(
                options.input,
                rank,
                world_size
            );

        AnalysisStats global;

        reduce_mpi_stats(
            local,
            global,
            rank,
            world_size
        );

        MPI_Barrier(
            MPI_COMM_WORLD
        );

        auto end =
            std::chrono::steady_clock::now();

        double local_seconds =
            std::chrono::duration<double>(
                end - start
            ).count();

        double maximum_seconds = 0.0;

        MPI_Reduce(
            &local_seconds,
            &maximum_seconds,
            1,
            MPI_DOUBLE,
            MPI_MAX,
            0,
            MPI_COMM_WORLD
        );

        if (rank == 0)
        {
            std::cout
                << "\n============================================\n"
                << "          CYBERSENTINEL - MPI\n"
                << "============================================\n"

                << "Dataset            : "
                << options.input
                << '\n'

                << "MPI processes      : "
                << world_size
                << '\n'

                << "OpenMP threads     : 1\n"

                << "--------------------------------------------\n";

            print_statistics(
                global,
                maximum_seconds
            );
        }
    }

    /*
     * ========================================
     * HYBRID MPI + OPENMP
     * ========================================
     */
    else if (
        options.mode == "hybrid"
    )
    {
        MPI_Barrier(
            MPI_COMM_WORLD
        );

        auto start =
            std::chrono::steady_clock::now();

        FileRange range =
            calculate_range(
                options.input,
                rank,
                world_size
            );

        std::vector<NetworkRecord>
            records =
                load_range(
                    options.input,
                    range
                );

        AnalysisStats local =
            analyse_openmp(
                records
            );

        AnalysisStats global;

        reduce_mpi_stats(
            local,
            global,
            rank,
            world_size
        );

        MPI_Barrier(
            MPI_COMM_WORLD
        );

        auto end =
            std::chrono::steady_clock::now();

        double local_seconds =
            std::chrono::duration<double>(
                end - start
            ).count();

        double maximum_seconds = 0.0;

        MPI_Reduce(
            &local_seconds,
            &maximum_seconds,
            1,
            MPI_DOUBLE,
            MPI_MAX,
            0,
            MPI_COMM_WORLD
        );

        if (rank == 0)
        {
            int threads =
                omp_get_max_threads();

            std::cout
                << "\n============================================\n"
                << "        CYBERSENTINEL - HYBRID\n"
                << "============================================\n"

                << "Dataset            : "
                << options.input
                << '\n'

                << "MPI processes      : "
                << world_size
                << '\n'

                << "OpenMP threads/rank: "
                << threads
                << '\n'

                << "Total workers      : "
                << world_size * threads
                << '\n'

                << "--------------------------------------------\n";

            print_statistics(
                global,
                maximum_seconds
            );
        }
    }

    /*
     * ========================================
     * ADAPTIVE HYBRID
     * ========================================
     */
    else if (
        options.mode == "adaptive"
    )
    {
        MPI_Barrier(
            MPI_COMM_WORLD
        );

        auto start =
            std::chrono::steady_clock::now();

        AnalysisStats global =
            run_adaptive(
                options.input,
                rank,
                world_size,
                options.chunks
            );

        MPI_Barrier(
            MPI_COMM_WORLD
        );

        auto end =
            std::chrono::steady_clock::now();

        double local_seconds =
            std::chrono::duration<double>(
                end - start
            ).count();

        double maximum_seconds = 0.0;

        MPI_Reduce(
            &local_seconds,
            &maximum_seconds,
            1,
            MPI_DOUBLE,
            MPI_MAX,
            0,
            MPI_COMM_WORLD
        );

        if (rank == 0)
        {
            std::cout
                << "\n============================================\n"
                << "     CYBERSENTINEL - ADAPTIVE HYBRID\n"
                << "============================================\n"

                << "Dataset            : "
                << options.input
                << '\n'

                << "MPI scheduler      : Rank 0\n"

                << "MPI worker ranks   : "
                << world_size - 1
                << '\n'

                << "OpenMP threads/rank: "
                << omp_get_max_threads()
                << '\n'

                << "Chunks/worker      : "
                << options.chunks
                << '\n'

                << "--------------------------------------------\n";

            print_statistics(
                global,
                maximum_seconds
            );

            std::cout
                << "\nAdaptive scheduler dynamically distributed "
                << (world_size - 1) *
                   options.chunks
                << " workload chunks.\n";
        }
    }
    else
    {
        if (rank == 0)
        {
            std::cout
                << "Unknown mode.\n";

            print_usage();
        }
    }

    MPI_Finalize();

    return 0;
}