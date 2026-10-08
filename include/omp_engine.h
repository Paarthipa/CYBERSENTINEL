#pragma once

#include "network_record.h"
#include "threat_detector.h"

#include <vector>

AnalysisStats run_openmp(
    const std::vector<NetworkRecord>& records
);