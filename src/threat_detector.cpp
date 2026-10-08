#include "threat_detector.h"

#include <algorithm>
#include <cmath>

static bool common_port(
    std::uint16_t port)
{
    return port == 22 ||
           port == 25 ||
           port == 53 ||
           port == 80 ||
           port == 123 ||
           port == 443;
}

bool is_candidate(
    const NetworkRecord& record)
{
    double connection_rate =
        static_cast<double>(
            record.packet_count
        ) /
        std::max(
            0.01,
            record.duration
        );

    /*
     * Stage 1:
     * Fast screening of every network flow.
     */
    return
        record.packet_count > 80 ||
        record.byte_count > 100000 ||
        record.success == 0 ||
        !common_port(record.destination_port) ||
        connection_rate > 100.0;
}

double calculate_threat_score(
    const NetworkRecord& record)
{
    /*
     * Stage 2:
     * More detailed threat analysis.
     */

    double packet_score =
        std::min(
            1.0,
            static_cast<double>(
                record.packet_count
            ) / 500.0
        );

    double byte_score =
        std::min(
            1.0,
            static_cast<double>(
                record.byte_count
            ) / 1000000.0
        );

    double connection_rate =
        static_cast<double>(
            record.packet_count
        ) /
        std::max(
            0.01,
            record.duration
        );

    double rate_score =
        std::min(
            1.0,
            connection_rate / 300.0
        );

    double port_score =
        common_port(
            record.destination_port
        )
        ? 0.0
        : 0.8;

    double failure_score =
        record.success ? 0.0 : 1.0;

    double score =
        0.15 * packet_score +
        0.25 * byte_score +
        0.25 * rate_score +
        0.15 * port_score +
        0.20 * failure_score;

    return std::clamp(
        score,
        0.0,
        1.0
    );
}

ThreatClass classify_score(
    double score)
{
    if (score < 0.30)
    {
        return ThreatClass::NORMAL;
    }

    if (score < 0.60)
    {
        return ThreatClass::LOW;
    }

    if (score < 0.80)
    {
        return ThreatClass::MEDIUM;
    }

    return ThreatClass::HIGH;
}

static void add_top_threat(
    AnalysisStats& stats,
    const Threat& threat)
{
    int existing = -1;

    for (int i = 0;
         i < stats.top_count;
         ++i)
    {
        if (stats.top[i].source_ip ==
            threat.source_ip)
        {
            existing = i;
            break;
        }
    }

    if (existing >= 0)
    {
        if (threat.score >
            stats.top[existing].score)
        {
            stats.top[existing] = threat;
        }
    }
    else if (stats.top_count < 10)
    {
        stats.top[
            stats.top_count++
        ] = threat;
    }
    else
    {
        int worst = 0;

        for (int i = 1; i < 10; ++i)
        {
            if (stats.top[i].score <
                stats.top[worst].score)
            {
                worst = i;
            }
        }

        if (threat.score >
            stats.top[worst].score)
        {
            stats.top[worst] = threat;
        }
    }

    std::sort(
        stats.top.begin(),
        stats.top.begin() +
            stats.top_count,
        [](const Threat& a,
           const Threat& b)
        {
            return a.score > b.score;
        }
    );
}

void analyse_record(
    AnalysisStats& stats,
    const NetworkRecord& record)
{
    stats.records++;

    /*
     * Stage 1.
     */
    if (!is_candidate(record))
    {
        stats.normal++;
        return;
    }

    stats.candidates++;

    /*
     * Stage 2.
     */
    double score =
        calculate_threat_score(record);

    ThreatClass classification =
        classify_score(score);

    switch (classification)
    {
        case ThreatClass::NORMAL:
            stats.normal++;
            break;

        case ThreatClass::LOW:
            stats.low++;
            break;

        case ThreatClass::MEDIUM:
            stats.medium++;
            break;

        case ThreatClass::HIGH:
            stats.high++;
            break;
    }

    add_top_threat(
        stats,
        {
            record.source_ip,
            score,
            record.destination_port
        }
    );
}

void merge_stats(
    AnalysisStats& destination,
    const AnalysisStats& source)
{
    destination.records +=
        source.records;

    destination.candidates +=
        source.candidates;

    destination.normal +=
        source.normal;

    destination.low +=
        source.low;

    destination.medium +=
        source.medium;

    destination.high +=
        source.high;

    for (int i = 0;
         i < source.top_count;
         ++i)
    {
        add_top_threat(
            destination,
            source.top[i]
        );
    }
}