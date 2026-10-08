#pragma once

#include <string>

double speedup(
    double sequential_time,
    double parallel_time
);

double efficiency(
    double speedup_value,
    int workers
);

double throughput(
    long long records,
    double seconds
);

std::string format_seconds(double seconds);