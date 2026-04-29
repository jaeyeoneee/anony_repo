// core/condshare.h — CondShare interface (Appendix A / Def 3.1).

#pragma once

#include "core/ess_backend.h"

namespace epochess {

inline std::vector<Share>
CondShare(const ESSConfig& cfg,
          const std::vector<uint64_t>& U,
          uint64_t epoch,
          const Poly& secret) {
    return cond_share(cfg, U, epoch, secret);
}

} 
