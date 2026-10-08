#pragma once

#include "network_record.h"

#include <string>
#include <vector>

struct FileRange {
    long long begin;
    long long end;
};

long long dataset_size(const std::string& path);

FileRange calculate_range(
    const std::string& path,
    int rank,
    int world_size
);

std::vector<NetworkRecord> load_range(
    const std::string& path,
    FileRange range
);

std::vector<NetworkRecord> load_all(
    const std::string& path
);