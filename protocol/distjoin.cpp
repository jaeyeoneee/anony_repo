// protocol/distjoin.cpp — Algorithm 1 implementation.

#include "protocol/distjoin.h"
#include "core/lagrange.h"
#include "core/hash_to_field.h"
#include "core/amd_tag.h"
#include "core/polyring.h"

#include <stdexcept>
#include <sstream>

namespace epochess {

namespace {

Metrics snapshot_delta(const Metrics& before, const Metrics& after) {
    Metrics d;
    d.total_msg_count = after.total_msg_count - before.total_msg_count;
    d.total_app_bytes = after.total_app_bytes - before.total_app_bytes;
    d.total_channel_bytes = after.total_channel_bytes - before.total_channel_bytes;
    return d;
}

} 

DistJoinResult
dist_join(const ESSConfig& cfg,
          uint64_t joiner_id,
          uint64_t epoch,
          const std::vector<Share>& committee_shares,
          Bus& bus,
          bool use_amd_tag) {
    DistJoinResult result;

    size_t committee_size = committee_shares.size();
    if (committee_size != static_cast<size_t>(cfg.k) + 1) {
        return result;
    }

    NTL::GF2E y_j = hash_to_field(joiner_id);

    // -------- Step 1: Feasibility test. --------
    std::vector<NTL::GF2E> y_points;
    for (const auto& sh : committee_shares) y_points.push_back(sh.y);
    for (const auto& yi : y_points) {
        if (yi == y_j) {
            // Joiner's eval point coincides with a committee member: abort.
            return result;
        }
    }
    auto lams_opt = lagrange_coefficients(y_points, y_j);
    if (!lams_opt) {
        return result;
    }
    const auto& lambdas = *lams_opt;

    // -------- Step 2: Zero-sum mask exchange. --------
    std::vector<std::vector<Poly>> rho_received(committee_size);
    std::vector<std::vector<Poly>> rho_sent(committee_size);
    for (size_t i = 0; i < committee_size; i++) {
        rho_received[i].resize(committee_size);
        rho_sent[i].resize(committee_size);
    }

    Metrics before = bus.metrics();
    (void)before;  

    std::vector<uint8_t> amd_key;
    if (use_amd_tag) amd_key = amd_keygen();

    for (size_t a = 0; a < committee_size; a++) {
        for (size_t b = 0; b < committee_size; b++) {
            if (a == b) continue;
            Poly rho = random_poly(cfg.L_sys);
            rho_sent[a][b] = rho;
            Envelope env;
            env.from = committee_shares[a].identity;
            env.to = committee_shares[b].identity;
            env.kind = MsgKind::DISTJOIN_MASK;
            env.payload = serialize(rho, cfg.L_sys);
            if (use_amd_tag) {
                env.tag = amd_tag(amd_key, env.payload, AMD_TAG_BYTES_DEFAULT);
            }
            bus.send(env);

            rho_received[b][a] = rho;
        }
    }

    for (const auto& sh : committee_shares) {
        (void)bus.receive(sh.identity);
    }

    std::vector<Poly> r(committee_size);
    for (size_t i = 0; i < committee_size; i++) {
        Poly acc;
        for (size_t a = 0; a < committee_size; a++) {
            if (a == i) continue;
            acc += rho_received[i][a];
        }
        for (size_t b = 0; b < committee_size; b++) {
            if (b == i) continue;
            acc -= rho_sent[i][b];
        }
        trunc_mod_xL(acc, cfg.L_sys);
        r[i] = acc;
    }

#ifndef NDEBUG
    {
        Poly zero_check;
        for (const auto& ri : r) zero_check += ri;
        trunc_mod_xL(zero_check, cfg.L_sys);
        if (!NTL::IsZero(zero_check)) {
            throw std::runtime_error("DistJoin: zero-sum mask invariant broken");
        }
    }
#endif

    // -------- Step 3: Masked contributions to joiner. --------
    std::vector<Poly> m(committee_size);
    for (size_t i = 0; i < committee_size; i++) {
        Poly scaled;
        long d = NTL::deg(committee_shares[i].sigma);
        for (long jx = 0; jx <= d; jx++) {
            NTL::GF2E c = NTL::coeff(committee_shares[i].sigma, jx) * lambdas[i];
            NTL::SetCoeff(scaled, jx, c);
        }
        Poly mi = scaled + r[i];
        trunc_mod_xL(mi, cfg.L_sys);
        m[i] = mi;

        Envelope env;
        env.from = committee_shares[i].identity;
        env.to = joiner_id;
        env.kind = MsgKind::DISTJOIN_MASKED;
        env.payload = serialize(mi, cfg.L_sys);
        if (use_amd_tag) {
            env.tag = amd_tag(amd_key, env.payload, AMD_TAG_BYTES_DEFAULT);
        }
        bus.send(env);
    }

    (void)bus.receive(joiner_id);

    // -------- Step 4: Joiner aggregation. --------
    Poly V_sys;
    for (const auto& mi : m) V_sys += mi;
    trunc_mod_xL(V_sys, cfg.L_sys);

    size_t L_j = per_identity_bound(joiner_id, cfg.k, cfg.lambda);
    if (L_j > cfg.L_sys) L_j = cfg.L_sys;
    Poly sigma_j = V_sys;
    trunc_mod_xL(sigma_j, L_j);

    Share out;
    out.identity = joiner_id;
    out.epoch = epoch;
    out.y = y_j;
    out.sigma = sigma_j;
    out.L_i = L_j;

    Metrics after = bus.metrics();
    Metrics d = snapshot_delta(before, after);

    result.success = true;
    result.share_for_joiner = out;
    result.retries = 0;
    result.msg_count = d.total_msg_count;
    result.app_bytes = d.total_app_bytes;
    result.channel_bytes = d.total_channel_bytes;
    return result;
}

}  
