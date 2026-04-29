// protocol/hardrefresh.h — Secret renewal via DKG (simplified).

#pragma once

#include "core/ess_backend.h"
#include "net/inproc_bus.h"

#include <cstdint>
#include <vector>

namespace epochess {

struct HardRefreshResult {
    bool success = false;
    std::vector<Share> new_shares;
    uint64_t msg_count = 0;
    uint64_t app_bytes = 0;
    uint64_t channel_bytes = 0;
};

// Each participant contributes a random secret fragment
HardRefreshResult
hard_refresh(const ESSConfig& cfg_next,
             const std::vector<uint64_t>& U_next,
             uint64_t epoch_next,
             Bus& bus);

}  
