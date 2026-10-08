#pragma once

#include "threat_detector.h"

#include <string>

AnalysisStats run_adaptive(
    const std::string& input,
    int rank,
    int world_size,
    int chunks_per_worker
);