// protocol/a2pe.cpp

#include "protocol/a2pe.h"

#include <unordered_map>

namespace epochess {

A2PEResult
run_a2pe(const std::vector<uint64_t>& overlap_committee_ids,
         const std::vector<uint64_t>& U_next,
         uint64_t epoch_next,
         unsigned k_next,
         Bus& bus) {
    A2PEResult r;
    Metrics before = bus.metrics();

    constexpr size_t ACK_BYTES = 80;

    for (uint64_t j : U_next) {
        for (uint64_t i : overlap_committee_ids) {
            Envelope env;
            env.from = j;
            env.to = i;
            env.kind = MsgKind::ACK_EPOCH;
            env.payload.resize(ACK_BYTES, 0);
            // Encode epoch at payload[0..8] for realism.
            for (int b = 0; b < 8; b++) {
                env.payload[b] = static_cast<uint8_t>((epoch_next >> (8 * b)) & 0xFF);
            }
            bus.send(env);
        }
    }

    std::unordered_map<uint64_t, uint64_t> ack_count;
    for (uint64_t i : overlap_committee_ids) {
        auto msgs = bus.receive(i);
        for (const auto& m : msgs) {
            if (m.kind == MsgKind::ACK_EPOCH) ack_count[i]++;
        }
    }

    r.safe_to_erase = true;
    uint64_t min_acks = U_next.size();
    for (uint64_t i : overlap_committee_ids) {
        uint64_t cnt = ack_count[i];
        if (cnt < min_acks) min_acks = cnt;
        if (cnt < k_next) r.safe_to_erase = false;
    }
    r.acks_received = min_acks;

    Metrics after = bus.metrics();
    r.msg_count = after.total_msg_count - before.total_msg_count;
    r.app_bytes = after.total_app_bytes - before.total_app_bytes;
    r.channel_bytes = after.total_channel_bytes - before.total_channel_bytes;
    return r;
}

}  
