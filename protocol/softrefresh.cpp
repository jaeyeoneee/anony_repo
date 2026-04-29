// protocol/softrefresh.cpp

#include "protocol/softrefresh.h"
#include "core/lagrange.h"
#include "core/polyring.h"
#include "core/hash_to_field.h"
#include "core/amd_tag.h"

#include <stdexcept>

namespace epochess {

SoftRefreshResult
soft_refresh(const ESSConfig& cfg_e,
             const ESSConfig& cfg_next,
             const std::vector<Share>& overlap_committee,
             const std::vector<uint64_t>& U_next,
             uint64_t epoch_next,
             Bus& bus,
             bool use_amd_tag) {
    SoftRefreshResult result;

    if (overlap_committee.size() != static_cast<size_t>(cfg_e.k)) {
        return result;
    }

    std::vector<NTL::GF2E> y_points;
    for (const auto& sh : overlap_committee) y_points.push_back(sh.y);
    NTL::GF2E zero;
    NTL::clear(zero);
    auto lams_opt = lagrange_coefficients(y_points, zero);
    if (!lams_opt) return result;

    Metrics before = bus.metrics();

    std::vector<uint8_t> amd_key;
    if (use_amd_tag) amd_key = amd_keygen();

    std::vector<std::vector<Share>> per_dealer_shares(overlap_committee.size());
    for (size_t i = 0; i < overlap_committee.size(); i++) {
        Poly scaled;
        long d = NTL::deg(overlap_committee[i].sigma);
        for (long jx = 0; jx <= d; jx++) {
            NTL::GF2E c = NTL::coeff(overlap_committee[i].sigma, jx) * (*lams_opt)[i];
            NTL::SetCoeff(scaled, jx, c);
        }
        Poly c_i = project(scaled, cfg_next.lambda);

        auto dealer_shares = cond_share(cfg_next, U_next, epoch_next, c_i);
        per_dealer_shares[i] = dealer_shares;

        for (const auto& q_ij : dealer_shares) {
            Envelope env;
            env.from = overlap_committee[i].identity;
            env.to = q_ij.identity;
            env.kind = MsgKind::SOFTREFRESH_SHARE;
            env.payload = serialize(q_ij.sigma, q_ij.L_i);
            if (use_amd_tag) {
                env.tag = amd_tag(amd_key, env.payload, AMD_TAG_BYTES_DEFAULT);
            }
            bus.send(env);
        }
    }

    for (uint64_t j_id : U_next) {
        (void)bus.receive(j_id);
    }

    result.new_shares.reserve(U_next.size());
    for (size_t j_idx = 0; j_idx < U_next.size(); j_idx++) {
        uint64_t j_id = U_next[j_idx];
        Poly agg;
        for (size_t i = 0; i < overlap_committee.size(); i++) {
            agg += per_dealer_shares[i][j_idx].sigma;
        }
        NTL::GF2E y_j = hash_to_field(j_id);
        size_t L_j = per_identity_bound(j_id, cfg_next.k, cfg_next.lambda);
        if (L_j > cfg_next.L_sys) L_j = cfg_next.L_sys;
        trunc_mod_xL(agg, L_j);

        Share out;
        out.identity = j_id;
        out.epoch = epoch_next;
        out.y = y_j;
        out.sigma = agg;
        out.L_i = L_j;
        result.new_shares.push_back(out);
    }

    Metrics after = bus.metrics();
    result.msg_count = after.total_msg_count - before.total_msg_count;
    result.app_bytes = after.total_app_bytes - before.total_app_bytes;
    result.channel_bytes = after.total_channel_bytes - before.total_channel_bytes;
    result.success = true;
    return result;
}

}  
