// adversary/byzantine.cpp

#include "adversary/byzantine.h"

namespace epochess {

ByzantineBus::ByzantineBus(Bus& inner,
                           std::set<uint64_t> malicious_ids,
                           ByzantineBehavior behavior)
    : inner_(inner),
      malicious_ids_(std::move(malicious_ids)),
      behavior_(behavior) {}

void ByzantineBus::send(const Envelope& e) {
    bool mal = (malicious_ids_.count(e.from) != 0);
    if (!mal || behavior_ == ByzantineBehavior::NONE) {
        inner_.send(e);
        return;
    }
    switch (behavior_) {
        case ByzantineBehavior::OMISSION: {
            return;
        }
        case ByzantineBehavior::GARBAGE: {
            Envelope e2 = e;
            for (auto& b : e2.payload) {
                b = static_cast<uint8_t>(rng_() & 0xFF);
            }
            inner_.send(e2);
            return;
        }
        case ByzantineBehavior::INCONSISTENT: {
            Envelope e2 = e;
            if (!e2.payload.empty()) {
                size_t idx = rng_() % e2.payload.size();
                e2.payload[idx] ^= static_cast<uint8_t>(rng_() & 0xFF);
            }
            inner_.send(e2);
            return;
        }
        case ByzantineBehavior::REPLAY: {
            if (!last_env_for_replay_.empty()) {
                Envelope e2 = last_env_for_replay_.back();
                e2.from = e.from;
                e2.to = e.to;
                inner_.send(e2);
            } else {
                inner_.send(e);
            }
            last_env_for_replay_.push_back(e);
            return;
        }
        default:
            inner_.send(e);
    }
}

std::vector<Envelope> ByzantineBus::receive(uint64_t to) {
    return inner_.receive(to);
}

}  
