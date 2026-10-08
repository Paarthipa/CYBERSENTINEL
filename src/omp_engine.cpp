#include "omp_engine.h"

#include <omp.h>
#include <vector>

AnalysisStats run_openmp(
    const std::vector<NetworkRecord>& records)
{
    int thread_count =
        omp_get_max_threads();

    std::vector<AnalysisStats>
        local_stats(thread_count);

    #pragma omp parallel
    {
        int thread_id =
            omp_get_thread_num();

        /*
         * Dynamic scheduling is intentional.
         *
         * Some network records require
         * more analysis than others.
         */
        #pragma omp for schedule(dynamic, 64)
        for (
            int i = 0;
            i < static_cast<int>(
                    records.size()
                );
            ++i)
        {
            analyse_record(
                local_stats[thread_id],
                records[i]
            );
        }
    }

    AnalysisStats total;

    for (const auto& stats :
         local_stats)
    {
        merge_stats(
            total,
            stats
        );
    }

    return total;
}