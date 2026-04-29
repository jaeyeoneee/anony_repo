// tests/test_distjoin.cpp

#include "core/field.h"
#include "core/ess_backend.h"
#include "protocol/distjoin.h"
#include "net/inproc_bus.h"

#include <cassert>
#include <cstdio>

using namespace epochess;

int main() {
    init_field(16);
    for (unsigned k : {3u, 5u, 10u}) {
        ESSConfig cfg{16, 16, 128, k};
        std::vector<uint64_t> U;
        for (uint64_t i = 1; i <= 30; i++) U.push_back(i);
        auto secret = random_secret(cfg.lambda);
        auto shares = cond_share(cfg, U, 1, secret);

        std::vector<Share> committee(shares.begin(), shares.begin() + k + 1);
        InProcBus bus;
        uint64_t joiner = 10000 + k;
        auto r = dist_join(cfg, joiner, 1, committee, bus);
        assert(r.success);

        // Expected messages: k(k+1) pairwise masks + (k+1) contributions.
        uint64_t expected = static_cast<uint64_t>(k) * (k + 1) + (k + 1);
        assert(r.msg_count == expected);

        // New share + k-1 originals should reconstruct.
        std::vector<Share> rec_set;
        rec_set.push_back(*r.share_for_joiner);
        // Pick k-1 more that are disjoint from committee.
        for (size_t i = k + 1; i < k + 1 + k - 1; i++) {
            rec_set.push_back(shares[i]);
        }
        auto rec = reconstruct(cfg, rec_set);
        assert(rec == project(secret, cfg.lambda));

        std::printf("[distjoin k=%u] msg=%lu bytes=%lu PASSED\n",
                    k, (unsigned long)r.msg_count, (unsigned long)r.app_bytes);
    }
    return 0;
}
