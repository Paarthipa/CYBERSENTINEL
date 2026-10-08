#pragma once

#include <cstdint>
#include <string>

struct NetworkRecord {
    std::uint32_t source_ip;
    std::uint32_t destination_ip;

    std::uint16_t source_port;
    std::uint16_t destination_port;

    std::uint32_t packet_count;
    std::uint64_t byte_count;

    double duration;

    std::uint8_t protocol;   // 0 = TCP, 1 = UDP
    std::uint8_t success;    // 1 = SUCCESS, 0 = FAILED
};

std::uint32_t ip_to_int(const std::string& ip);
std::string int_to_ip(std::uint32_t ip);

bool parse_csv_record(
    const std::string& line,
    NetworkRecord& record
);