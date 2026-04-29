// protocol/softrefresh.h — Algorithm 2: secret-preserving epoch transition.

#pragma once

#include "core/ess_backend.h"
#include "net/inproc_bus.h"

#include <cstdint>
#include <vector>
#include <optional>

namespace epochess {

struct SoftRefreshResult {
    bool success = false;
    std::vector<Share> new_shares;  
    uint64_t msg_count = 0;
    uint64_t app_bytes = 0;
    uint64_t channel_bytes = 0;
};

// Execute SoftRefresh. 
SoftRefreshResult
soft_refresh(const ESSConfig& cfg_e,          
             const ESSConfig& cfg_next,     
             const std::vector<Share>& overlap_committee,  
             const std::vector<uint64_t>& U_next,
             uint64_t epoch_next,
             Bus& bus,
             bool use_amd_tag = false);

} 
