// protocol/a2pe.h — Asynchronous Two-Phase Erasure (paper Section 4.3.3).
//
// Three stages:
//   1. Prepare  — dealer distributes contributions, retains epoch-e state
//   2. Acknowledge — each recipient P_j broadcasts a signed Ack(e+1)_j upon
//                    successful installation
//   3. Commit/Erase — honest parties erase st_i^(e) only after collecting
//                     >= k_{e+1} valid Acks
//
// This is essentially a liveness barrier; for our simulations we implement
// a simplified synchronous variant that counts ack bytes for metrics and
// validates the barrier condition, but doesn't simulate true asynchrony
// (that is the subject of Exp 6 with the loss-rate mechanism in Bus).

#pragma once

#include "net/inproc_bus.h"

#include <cstdint>
#include <vector>

namespace epochess {

struct A2PEResult {
    bool safe_to_erase = false;
    uint64_t acks_received = 0;
    uint64_t app_bytes = 0;
    uint64_t channel_bytes = 0;
    uint64_t msg_count = 0;
};

// Have each recipient in U_next broadcast an Ack for epoch_next to each
// dealer in overlap_committee_ids. After collection, check whether every
// dealer received >= k_next acknowledgments.
A2PEResult
run_a2pe(const std::vector<uint64_t>& overlap_committee_ids,
         const std::vector<uint64_t>& U_next,
         uint64_t epoch_next,
         unsigned k_next,
         Bus& bus);

}  // namespace epochess
