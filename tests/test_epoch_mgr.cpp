// tests/test_epoch_mgr.cpp — end-to-end lifecycle test.

#include "protocol/epoch_mgr.h"
#include "net/inproc_bus.h"

#include <cassert>
#include <cstdio>

using namespace epochess;

int main() {
    init_field(16);
    ESSConfig cfg{16, 16, 128, 5};
    std::vector<uint64_t> U;
    for (uint64_t i = 1; i <= 20; i++) U.push_back(i);

    InProcBus bus;
    EpochManager mgr(cfg, U, bus);
    assert(mgr.epoch() == 1);
    auto original_secret = mgr.current_segment_secret();

    // DistJoin a few parties.
    for (uint64_t id : {21ull, 22ull, 23ull}) {
        bool ok = mgr.dist_join_one(id);
        assert(ok);
    }
    assert(mgr.active_set().size() == 23);

    // Verify secret reconstruction via a committee including the new joiners.
    auto rec1 = mgr.reconstruct_epoch_secret({1, 21, 22, 23, 10});
    assert(rec1 == project(original_secret, cfg.lambda));

    // Soft transition: add more, revoke one.
    bool ok = mgr.soft_transition({30, 31}, {22}, cfg.k);
    assert(ok);
    assert(mgr.epoch() == 2);

    auto rec2 = mgr.reconstruct_epoch_secret({1, 2, 30, 31, 11});
    assert(rec2 == project(original_secret, cfg.lambda));

    // Hard transition: same threshold, but renews secret.
    ok = mgr.hard_transition({}, {}, cfg.k);
    assert(ok);
    assert(mgr.epoch() == 3);
    auto new_secret = mgr.current_segment_secret();
    assert(new_secret != original_secret);

    std::printf("[epoch_mgr] PASSED\n");
    return 0;
}
