// net/metrics.h — Communication metrics.

#pragma once

#include <cstdint>
#include <cstddef>
#include <array>

namespace epochess {

// Keep this array in sync with MsgKind enum values.
constexpr size_t NUM_MSG_KINDS = 6;

struct Metrics {
    // Per-kind counts.
    std::array<uint64_t, NUM_MSG_KINDS> msg_count{};
    std::array<uint64_t, NUM_MSG_KINDS> app_bytes{};
    std::array<uint64_t, NUM_MSG_KINDS> channel_bytes{};

    // Aggregates.
    uint64_t total_msg_count = 0;
    uint64_t total_app_bytes = 0;
    uint64_t total_channel_bytes = 0;
    uint64_t dropped_count = 0;
};

}
