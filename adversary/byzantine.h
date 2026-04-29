// adversary/byzantine.h — Byzantine fault injection.
//
// Supports four attack types:
//   - OMISSION: drop outgoing messages
//   - GARBAGE: replace payload with random bytes
//   - INCONSISTENT: send different payloads to different recipients (equivocation)
//   - REPLAY: replay previous message
//
// Used in Exp 7 to measure AMD tag detection rates.

#pragma once

#include "net/inproc_bus.h"

#include <cstdint>
#include <set>
#include <vector>

namespace epochess {

enum class ByzantineBehavior {
    NONE,
    OMISSION,
    GARBAGE,
    INCONSISTENT,
    REPLAY
};

// Wrapping Bus that corrupts messages from a set of "malicious" senders.
class ByzantineBus : public Bus {
public:
    ByzantineBus(Bus& inner,
                 std::set<uint64_t> malicious_ids,
                 ByzantineBehavior behavior);

    void send(const Envelope& e) override;
    std::vector<Envelope> receive(uint64_t to) override;
    const Metrics& metrics() const override { return inner_.metrics(); }
    void reset_metrics() override { inner_.reset_metrics(); }
    void set_loss_rate(double r) override { inner_.set_loss_rate(r); }
    void set_rng_seed(uint64_t s) override {
        inner_.set_rng_seed(s);
        rng_.seed(s ^ 0xDEADBEEF);
    }

private:
    Bus& inner_;
    std::set<uint64_t> malicious_ids_;
    ByzantineBehavior behavior_;
    std::mt19937_64 rng_{0xB4D};
    std::vector<Envelope> last_env_for_replay_;
};

}  // namespace epochess
