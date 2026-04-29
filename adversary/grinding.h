// adversary/grinding.h — Identity grinding attack.

#pragma once

#include "core/field.h"
#include <cstdint>
#include <vector>

namespace epochess {

// Given a target set of y-values (committee), find a joiner identity whose y_j collides with one of them.
uint64_t grind_collision(const std::vector<NTL::GF2E>& committee_y,
                         uint64_t budget,
                         uint64_t defense_nonce = 0);

// count how many trials needed to find first collision
struct GrindStats {
    uint64_t trials;
    bool found;
};
GrindStats grind_count(const std::vector<NTL::GF2E>& committee_y,
                       uint64_t budget,
                       uint64_t defense_nonce = 0);

} 
