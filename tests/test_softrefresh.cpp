// tests/test_softrefresh.cpp

#include "core/field.h"
#include "core/ess_backend.h"
#include "protocol/softrefresh.h"
#include "net/inproc_bus.h"

#include <cassert>
#include <cstdio>

using namespace epochess;

int main() {
    init_field(16);
    unsigned k = 5;
    ESSConfig cfg{16, 16, 128, k};
    std::vector<uint64_t> U;
    for (uint64_t i = 1; i <= 20; i++) U.push_back(i);
    auto secret = random_secret(cfg.lambda);
    auto shares = cond_share(cfg, U, 1, secret);

    // Soft transition: revoke {3, 5}, add {21, 22, 23}.
    std::vector<uint64_t> U_next;
    for (uint64_t i = 1; i <= 20; i++) {
        if (i == 3 || i == 5) continue;
        U_next.push_back(i);
    }
    U_next.push_back(21);
    U_next.push_back(22);
    U_next.push_back(23);

    // Overlap committee: first k surviving shares.
    std::vector<Share> overlap;
    for (uint64_t id : {1ull, 2ull, 4ull, 6ull, 7ull}) {
        for (const auto& sh : shares) if (sh.identity == id) overlap.push_back(sh);
    }
    assert(overlap.size() == k);

    InProcBus bus;
    ESSConfig cfg_next = cfg;  // same threshold
    auto r = soft_refresh(cfg, cfg_next, overlap, U_next, 2, bus);
    assert(r.success);
    assert(r.new_shares.size() == U_next.size());

    // Verify: any k of the new shares reconstruct the *same* secret.
    std::vector<Share> committee(r.new_shares.begin(), r.new_shares.begin() + k);
    auto rec = reconstruct(cfg_next, committee);
    assert(rec == project(secret, cfg.lambda));

    std::printf("[softrefresh] msg=%lu bytes=%lu PASSED\n",
                (unsigned long)r.msg_count, (unsigned long)r.app_bytes);
    return 0;
}
