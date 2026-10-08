#pragma once

#include "network_record.h"
#include "threat_detector.h"

#include <vector>

AnalysisStats analyse_sequential(
    const std::vector<NetworkRecord>& records
);

AnalysisStats analyse_openmp(
    const std::vector<NetworkRecord>& records
);