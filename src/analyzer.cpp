#include "analyzer.h"
#include "omp_engine.h"

AnalysisStats analyse_sequential(
    const std::vector<NetworkRecord>& records)
{
    AnalysisStats stats;

    for (const auto& record : records)
    {
        analyse_record(stats, record);
    }

    return stats;
}

AnalysisStats analyse_openmp(
    const std::vector<NetworkRecord>& records)
{
    return run_openmp(records);
}