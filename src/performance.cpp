#include "performance.h"

#include <iomanip>
#include <sstream>

double speedup(
    double sequential_time,
    double parallel_time)
{
    if (parallel_time <= 0.0)
    {
        return 0.0;
    }

    return sequential_time /
           parallel_time;
}

double efficiency(
    double speedup_value,
    int workers)
{
    if (workers <= 0)
    {
        return 0.0;
    }

    return (
        speedup_value /
        static_cast<double>(workers)
    ) * 100.0;
}

double throughput(
    long long records,
    double seconds)
{
    if (seconds <= 0.0)
    {
        return 0.0;
    }

    return static_cast<double>(
        records
    ) / seconds;
}

std::string format_seconds(
    double seconds)
{
    std::ostringstream output;

    output
        << std::fixed
        << std::setprecision(4)
        << seconds;

    return output.str();
}