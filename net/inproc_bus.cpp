// net/inproc_bus.cpp

#include "net/inproc_bus.h"

namespace epochess {

namespace {
constexpr size_t CHANNEL_OVERHEAD = 32;

size_t kind_index(MsgKind k) {
    auto v = static_cast<uint8_t>(k);
    if (v >= NUM_MSG_KINDS) v = 0;  // UNKNOWN bucket
    return v;
}
}  

InProcBus::InProcBus() = default;

void InProcBus::send(const Envelope& e) {
    std::lock_guard<std::mutex> lk(mu_);

    size_t idx = kind_index(e.kind);
    size_t app = e.payload.size();
    size_t ch = app + e.tag.size() + CHANNEL_OVERHEAD;

    metrics_.msg_count[idx]++;
    metrics_.app_bytes[idx] += app;
    metrics_.channel_bytes[idx] += ch;
    metrics_.total_msg_count++;
    metrics_.total_app_bytes += app;
    metrics_.total_channel_bytes += ch;

    if (loss_rate_ > 0.0) {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        if (dist(rng_) < loss_rate_) {
            metrics_.dropped_count++;
            return;
        }
    }

    queues_[e.to].push(e);
}

std::vector<Envelope> InProcBus::receive(uint64_t to) {
    std::lock_guard<std::mutex> lk(mu_);
    std::vector<Envelope> out;
    auto it = queues_.find(to);
    if (it == queues_.end()) return out;
    auto& q = it->second;
    while (!q.empty()) {
        out.push_back(std::move(q.front()));
        q.pop();
    }
    return out;
}

void InProcBus::set_loss_rate(double rate) {
    std::lock_guard<std::mutex> lk(mu_);
    loss_rate_ = rate;
}

void InProcBus::set_rng_seed(uint64_t seed) {
    std::lock_guard<std::mutex> lk(mu_);
    rng_.seed(seed);
}

}  