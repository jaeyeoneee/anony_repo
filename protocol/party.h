// protocol/party.h

#pragma once

#include "core/ess_backend.h"

#include <cstdint>
#include <optional>

namespace epochess {

enum class PartyStatus { UNINITIALIZED, JOINED, IN_REFRESH, REVOKED };

struct PartyState {
    uint64_t id = 0;
    uint64_t epoch = 0;
    NTL::GF2E y;             
    std::optional<Share> share;  
    PartyStatus status = PartyStatus::UNINITIALIZED;
};

}  
