#include "dataset.h"

#include <fstream>
#include <string>

long long dataset_size(
    const std::string& path)
{
    std::ifstream file(
        path,
        std::ios::binary | std::ios::ate
    );

    if (!file)
    {
        return -1;
    }

    return static_cast<long long>(
        file.tellg()
    );
}

FileRange calculate_range(
    const std::string& path,
    int rank,
    int world_size)
{
    long long size =
        dataset_size(path);

    long long base =
        size / world_size;

    long long begin =
        static_cast<long long>(rank) * base;

    long long end =
        (rank == world_size - 1)
        ? size
        : static_cast<long long>(rank + 1) * base;

    return {begin, end};
}

std::vector<NetworkRecord> load_range(
    const std::string& path,
    FileRange range)
{
    std::vector<NetworkRecord> records;

    std::ifstream file(
        path,
        std::ios::binary
    );

    if (!file)
    {
        return records;
    }

    // Skip CSV header for rank 0.
    if (range.begin == 0)
    {
        std::string header;
        std::getline(file, header);
    }
    else
    {
        // Move to approximate byte boundary.
        file.seekg(range.begin);

        // Discard partial line.
        std::string partial;
        std::getline(file, partial);
    }

    std::string line;

    while (true)
    {
        long long position =
            static_cast<long long>(
                file.tellg()
            );

        if (position < 0 ||
            position >= range.end)
        {
            break;
        }

        if (!std::getline(file, line))
        {
            break;
        }

        NetworkRecord record{};

        if (parse_csv_record(line, record))
        {
            records.push_back(record);
        }
    }

    return records;
}

std::vector<NetworkRecord> load_all(
    const std::string& path)
{
    return load_range(
        path,
        {0, dataset_size(path)}
    );
}