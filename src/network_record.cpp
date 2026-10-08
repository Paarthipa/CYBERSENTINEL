#include "network_record.h"

#include <sstream>

std::uint32_t ip_to_int(const std::string& ip)
{
    unsigned a = 0;
    unsigned b = 0;
    unsigned c = 0;
    unsigned d = 0;

    char dot1 = 0;
    char dot2 = 0;
    char dot3 = 0;

    std::stringstream ss(ip);

    ss >> a >> dot1
       >> b >> dot2
       >> c >> dot3
       >> d;

    if (!ss ||
        dot1 != '.' ||
        dot2 != '.' ||
        dot3 != '.')
    {
        return 0;
    }

    return (a << 24) |
           (b << 16) |
           (c << 8) |
           d;
}

std::string int_to_ip(std::uint32_t ip)
{
    std::ostringstream out;

    out << ((ip >> 24) & 255)
        << "."
        << ((ip >> 16) & 255)
        << "."
        << ((ip >> 8) & 255)
        << "."
        << (ip & 255);

    return out.str();
}

bool parse_csv_record(
    const std::string& line,
    NetworkRecord& record)
{
    std::string columns[10];

    std::size_t start = 0;

    for (int i = 0; i < 9; ++i)
    {
        std::size_t position =
            line.find(',', start);

        if (position == std::string::npos)
        {
            return false;
        }

        columns[i] =
            line.substr(
                start,
                position - start
            );

        start = position + 1;
    }

    columns[9] = line.substr(start);

    try
    {
        record.source_ip =
            ip_to_int(columns[1]);

        record.destination_ip =
            ip_to_int(columns[2]);

        record.source_port =
            static_cast<std::uint16_t>(
                std::stoul(columns[3])
            );

        record.destination_port =
            static_cast<std::uint16_t>(
                std::stoul(columns[4])
            );

        record.protocol =
            columns[5] == "UDP" ? 1 : 0;

        record.packet_count =
            static_cast<std::uint32_t>(
                std::stoul(columns[6])
            );

        record.byte_count =
            std::stoull(columns[7]);

        record.duration =
            std::stod(columns[8]);

        record.success =
            columns[9] == "SUCCESS" ? 1 : 0;

        return true;
    }
    catch (...)
    {
        return false;
    }
}