// protocol/hardrefresh.cpp

#include "protocol/hardrefresh.h"
#include "core/polyring.h"

namespace epochess {

HardRefreshResult
hard_refresh(const ESSConfig& cfg_next,
             const std::vector<uint64_t>& U_next,
             uint64_t epoch_next,
             Bus& bus) {
    HardRefreshResult result;
    Metrics before = bus.metrics();

    Poly joint_secret;
    size_t frag_bytes = serialized_bytes(cfg_next.lambda);

    for (size_t i = 0; i < U_next.size(); i++) {
        Poly frag = random_poly(cfg_next.lambda);
        joint_secret += frag;
        trunc_mod_xL(joint_secret, cfg_next.lambda);

        for (size_t j = 0; j < U_next.size(); j++) {
            if (i == j) continue;
            Envelope env;
            env.from = U_next[i];
            env.to = U_next[j];
            env.kind = MsgKind::HARDREFRESH;
            env.payload.resize(frag_bytes, 0);  // dummy content; size is what matters
            bus.send(env);
        }
    }

    for (uint64_t id : U_next) (void)bus.receive(id);

    result.new_shares = cond_share(cfg_next, U_next, epoch_next, joint_secret);

    Metrics after = bus.metrics();
    result.msg_count = after.total_msg_count - before.total_msg_count;
    result.app_bytes = after.total_app_bytes - before.total_app_bytes;
    result.channel_bytes = after.total_channel_bytes - before.total_channel_bytes;
    result.success = true;
    return result;
}

}  
