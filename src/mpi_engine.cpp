#include "mpi_engine.h"

#include <mpi.h>

AnalysisStats run_mpi(
    const std::string& input,
    int rank,
    int world_size)
{
    FileRange range =
        calculate_range(
            input,
            rank,
            world_size
        );

    std::vector<NetworkRecord>
        records =
            load_range(
                input,
                range
            );

    AnalysisStats local;

    for (const auto& record :
         records)
    {
        analyse_record(
            local,
            record
        );
    }

    return local;
}

void reduce_mpi_stats(
    const AnalysisStats& local,
    AnalysisStats& global,
    int rank,
    int world_size)
{
    long long send_values[6] =
    {
        local.records,
        local.candidates,
        local.normal,
        local.low,
        local.medium,
        local.high
    };

    long long receive_values[6] =
    {
        0, 0, 0, 0, 0, 0
    };

    MPI_Reduce(
        send_values,
        receive_values,
        6,
        MPI_LONG_LONG,
        MPI_SUM,
        0,
        MPI_COMM_WORLD
    );

    if (rank == 0)
    {
        global.records =
            receive_values[0];

        global.candidates =
            receive_values[1];

        global.normal =
            receive_values[2];

        global.low =
            receive_values[3];

        global.medium =
            receive_values[4];

        global.high =
            receive_values[5];
    }

    /*
     * Gather top threats from every rank.
     */
    std::vector<Threat> gathered;

    if (rank == 0)
    {
        gathered.resize(
            world_size * 10
        );
    }

    MPI_Gather(
        local.top.data(),
        10 * sizeof(Threat),
        MPI_BYTE,

        rank == 0
            ? gathered.data()
            : nullptr,

        10 * sizeof(Threat),
        MPI_BYTE,

        0,
        MPI_COMM_WORLD
    );

    if (rank == 0)
    {
        global.top_count = 0;

        for (int rank_id = 0;
             rank_id < world_size;
             ++rank_id)
        {
            for (int i = 0; i < 10; ++i)
            {
                Threat threat =
                    gathered[
                        rank_id * 10 + i
                    ];

                if (
                    threat.source_ip == 0 &&
                    threat.score == 0.0
                )
                {
                    continue;
                }

                AnalysisStats temporary;

                temporary.top_count = 1;
                temporary.top[0] =
                    threat;

                merge_stats(
                    global,
                    temporary
                );
            }
        }
    }
}