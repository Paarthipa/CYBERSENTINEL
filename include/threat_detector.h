#pragma once

#include "network_record.h"

#include <array>
#include <cstdint>

enum class ThreatClass {
    NORMAL,
    LOW,
    MEDIUM,
    HIGH
};

struct Threat {
    std::uint32_t source_ip;
    double score;
    std::uint16_t destination_port;
};

struct AnalysisStats {
    long long records = 0;
    long long candidates = 0;

    long long normal = 0;
    long long low = 0;
    long long medium = 0;
    long long high = 0;

    std::array<Threat, 10> top{};
    int top_count = 0;
};

bool is_candidate(const NetworkRecord& record);

double calculate_threat_score(
    const NetworkRecord& record
);

ThreatClass classify_score(double score);

void analyse_record(
    AnalysisStats& stats,
    const NetworkRecord& record
);

void merge_stats(
    AnalysisStats& destination,
    const AnalysisStats& source
);