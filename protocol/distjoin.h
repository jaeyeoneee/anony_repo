// protocol/distjoin.h — Algorithm 1: dealerless in-epoch issuance.

#pragma once

#include "core/ess_backend.h"
#include "net/inproc_bus.h"

#include <cstdint>
#include <vector>
#include <optional>

namespace epochess {

struct DistJoinResult {
    bool success = false;
    std::optional<Share> share_for_joiner;
    uint64_t retries = 0;               // Number of committee retries performed
    uint64_t app_bytes = 0;             // Application-layer bytes exchanged
    uint64_t channel_bytes = 0;         // Channel-layer bytes exchanged
    uint64_t msg_count = 0;             // Total messages sent
};

// Execute DistJoin in-process, using the given Bus for metrics and message routing.
DistJoinResult
dist_join(const ESSConfig& cfg,
          uint64_t joiner_id,
          uint64_t epoch,
          const std::vector<Share>& committee_shares,  // size exactly k+1
          Bus& bus,
          bool use_amd_tag = false);

}  
