// net/inproc_bus.h — In-process message bus with metrics.
//
// The Bus models authenticated, confidential, point-to-point channels
// between parties. All transmissions are counted (bytes, messages) so
// experiments can report exact communication cost.
//
// For distributed experiments (Exp 5-9), this in-process bus is replaced by
// a Boost.Asio TCP-based bus exposing the same interface. For Exp 1-4, 10
// the in-process bus is sufficient because those claims are about pure
// communication accounting and cryptographic correctness.

#pragma once

#include "net/metrics.h"

#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <vector>
#include <unordered_map>
#include <random>

namespace epochess {

// Message type tags for metrics categorization.
enum class MsgKind : uint8_t {
    UNKNOWN = 0,
    DISTJOIN_MASK,      // rho_{a->b}
    DISTJOIN_MASKED,    // m_i sent to joiner
    SOFTREFRESH_SHARE,  // q_{i->j}
    ACK_EPOCH,          // A-2PE Ack
    HARDREFRESH,        // DKG contribution
    OTHER
};

struct Envelope {
    uint64_t from;
    uint64_t to;
    MsgKind kind;
    std::vector<uint8_t> payload;  // raw serialized bytes (app-layer)
    std::vector<uint8_t> tag;      // optional AMD tag
};

// Bus interface.
class Bus {
public:
    virtual ~Bus() = default;

    // Send an envelope. Records metrics at application layer.
    virtual void send(const Envelope& e) = 0;

    // Drain messages for party `to` (returns and clears the queue).
    virtual std::vector<Envelope> receive(uint64_t to) = 0;

    // Metrics accessor.
    virtual const Metrics& metrics() const = 0;
    virtual void reset_metrics() = 0;

    // Optional: simulate packet loss / reorder (for Exp 6).
    virtual void set_loss_rate(double rate) = 0;
    virtual void set_rng_seed(uint64_t seed) = 0;
};

// Concrete in-process implementation.
class InProcBus : public Bus {
public:
    InProcBus();

    void send(const Envelope& e) override;
    std::vector<Envelope> receive(uint64_t to) override;
    const Metrics& metrics() const override { return metrics_; }
    void reset_metrics() override { metrics_ = Metrics{}; }
    void set_loss_rate(double rate) override;
    void set_rng_seed(uint64_t seed) override;

private:
    std::mutex mu_;
    std::unordered_map<uint64_t, std::queue<Envelope>> queues_;
    Metrics metrics_;
    double loss_rate_ = 0.0;
    std::mt19937_64 rng_{0xC0DEFA1LU};
};

}  // namespace epochess
