#pragma once

#include "dataset.h"
#include "threat_detector.h"

#include <string>

AnalysisStats run_mpi(
    const std::string& input,
    int rank,
    int world_size
);

void reduce_mpi_stats(
    const AnalysisStats& local,
    AnalysisStats& global,
    int rank,
    int world_size
);