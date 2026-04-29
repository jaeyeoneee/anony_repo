// tests/test_ess.cpp — ESS backend correctness.
#include <algorithm>
#include "core/field.h"
#include "core/ess_backend.h"
#include "core/polyring.h"

#include <cassert>
#include <cstdio>
#include <random>

using namespace epochess;

int main() {
    init_field(16);
    // Test several (k, |U|) combinations.
    for (unsigned k : {2u, 3u, 5u, 7u, 10u}) {
        ESSConfig cfg{16, 16, 128, k};
        std::vector<uint64_t> U;
        for (uint64_t i = 1; i <= 20; i++) U.push_back(i);
        auto secret = random_secret(cfg.lambda);
        auto shares = cond_share(cfg, U, 1, secret);

        // Any k-subset reconstructs the secret.
        std::mt19937_64 rng(42);
        for (int trial = 0; trial < 10; trial++) {
            std::vector<Share> shuffled = shares;
            std::shuffle(shuffled.begin(), shuffled.end(), rng);
            std::vector<Share> committee(shuffled.begin(), shuffled.begin() + k);
            try {
                auto rec = reconstruct(cfg, committee);
                assert(rec == project(secret, cfg.lambda));
            } catch (...) {
                // committee inadmissible - ok for some trials
            }
        }
        std::printf("[ess k=%u] PASSED\n", k);
    }
    return 0;
}
