// adversary/grinding.cpp

#include "adversary/grinding.h"
#include "core/hash_to_field.h"
#include <unordered_set>

namespace epochess {

uint64_t grind_collision(const std::vector<NTL::GF2E>& committee_y,
                         uint64_t budget,
                         uint64_t defense_nonce) {
    std::unordered_set<uint64_t> target_set;
    for (const auto& y : committee_y) target_set.insert(to_uint(y));

    uint64_t base = 1'000'000'000ull;
    for (uint64_t t = 0; t < budget; t++) {
        uint64_t candidate = base + t;
        auto y = hash_to_field(candidate, defense_nonce);
        if (target_set.count(to_uint(y))) return candidate;
    }
    return 0;  
}

GrindStats grind_count(const std::vector<NTL::GF2E>& committee_y,
                       uint64_t budget,
                       uint64_t defense_nonce) {
    std::unordered_set<uint64_t> target_set;
    for (const auto& y : committee_y) target_set.insert(to_uint(y));
    GrindStats s{0, false};
    uint64_t base = 1'000'000'000ull;
    for (uint64_t t = 0; t < budget; t++) {
        s.trials = t + 1;
        uint64_t candidate = base + t;
        auto y = hash_to_field(candidate, defense_nonce);
        if (target_set.count(to_uint(y))) {
            s.found = true;
            return s;
        }
    }
    return s;
}

} 
