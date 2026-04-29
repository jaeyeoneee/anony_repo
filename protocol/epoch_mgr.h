// protocol/epoch_mgr.h

#pragma once

#include "core/ess_backend.h"
#include "net/inproc_bus.h"

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace epochess {

class EpochManager {
public:
    EpochManager(const ESSConfig& cfg,
                 const std::vector<uint64_t>& initial_U,
                 Bus& bus);

    // Current epoch number.
    uint64_t epoch() const { return epoch_; }

    // Current active set U^(e).
    const std::vector<uint64_t>& active_set() const { return U_; }

    // Current threshold k^(e).
    unsigned threshold() const { return cfg_.k; }

    // Retrieve share for given identity at current epoch.
    const Share* get_share(uint64_t id) const;

    // Issue (DistJoin) a new party at current epoch.
    bool dist_join_one(uint64_t joiner_id);

    // Transition to epoch e+1 via SoftRefresh.
    bool soft_transition(const std::vector<uint64_t>& J_next,
                         const std::vector<uint64_t>& R_next,
                         unsigned k_next);

    // Transition via HardRefresh (secret renewal).
    bool hard_transition(const std::vector<uint64_t>& J_next,
                         const std::vector<uint64_t>& R_next,
                         unsigned k_next);

    // Verify that k arbitrary shares reconstruct the same secret as the stored epoch-1 secret.
    Poly reconstruct_epoch_secret(const std::vector<uint64_t>& committee_ids) const;

    // Access the secret stored at the start of each segment.
    const Poly& current_segment_secret() const { return segment_secret_; }

private:
    ESSConfig cfg_;
    Bus& bus_;
    uint64_t epoch_ = 0;
    std::vector<uint64_t> U_;
    std::unordered_map<uint64_t, Share> shares_;
    Poly segment_secret_;  // for test invariant checks

    void install_shares(const std::vector<Share>& shs);
};

}  
