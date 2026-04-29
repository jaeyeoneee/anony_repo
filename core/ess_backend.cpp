// core/ess_backend.cpp

#include "core/ess_backend.h"
#include "core/hash_to_field.h"
#include "core/lagrange.h"

#include <NTL/GF2E.h>
#include <NTL/GF2EX.h>
#include <stdexcept>

namespace epochess {

Poly random_secret(size_t lambda) {
    return random_poly(lambda);
}

namespace {

Poly build_share_polynomial(const std::vector<Poly>& coeffs,
                            const NTL::GF2E& y,
                            size_t L_sys) {
    Poly acc = coeffs.back();  
    for (long j = static_cast<long>(coeffs.size()) - 2; j >= 0; j--) {
        Poly next;
        long d = NTL::deg(acc);
        for (long i = 0; i <= d; i++) {
            NTL::GF2E c = NTL::coeff(acc, i) * y;
            NTL::SetCoeff(next, i, c);
        }
        next += coeffs[j];
        acc = next;
        trunc_mod_xL(acc, L_sys);
    }
    return acc;
}

}  

std::vector<Share>
cond_share(const ESSConfig& cfg,
           const std::vector<uint64_t>& U,
           uint64_t epoch,
           const Poly& secret) {
    if (cfg.k < 1) throw std::invalid_argument("threshold k must be >= 1");
    std::vector<Poly> coeffs(cfg.k);
    coeffs[0] = secret;
    trunc_mod_xL(coeffs[0], cfg.lambda); 
    for (unsigned j = 1; j < cfg.k; j++) {
        coeffs[j] = random_poly(cfg.L_sys);
    }

    std::vector<Share> shares;
    shares.reserve(U.size());
    for (uint64_t id : U) {
        NTL::GF2E y = hash_to_field(id);
        Poly sigma = build_share_polynomial(coeffs, y, cfg.L_sys);
        size_t L_i = per_identity_bound(id, cfg.k, cfg.lambda);
        if (L_i > cfg.L_sys) L_i = cfg.L_sys;
        trunc_mod_xL(sigma, L_i);
        shares.push_back({id, epoch, y, sigma, L_i});
    }
    return shares;
}

Poly reconstruct(const ESSConfig& cfg, const std::vector<Share>& shares) {
    if (shares.size() < cfg.k) {
        throw std::runtime_error("insufficient shares for reconstruction");
    }
    std::vector<Share> committee(shares.begin(), shares.begin() + cfg.k);
    std::vector<NTL::GF2E> y_points;
    for (const auto& sh : committee) y_points.push_back(sh.y);

    NTL::GF2E zero;  
    NTL::clear(zero);
    auto lams = lagrange_coefficients(y_points, zero);
    if (!lams) throw std::runtime_error("committee inadmissible at reconstruct");

    Poly acc;
    for (size_t m = 0; m < committee.size(); m++) {
        Poly scaled;
        long d = NTL::deg(committee[m].sigma);
        for (long j = 0; j <= d; j++) {
            NTL::GF2E c = NTL::coeff(committee[m].sigma, j) * (*lams)[m];
            NTL::SetCoeff(scaled, j, c);
        }
        acc += scaled;
    }
    trunc_mod_xL(acc, cfg.lambda);
    return acc;
}

Poly eval_linear(const ESSConfig& cfg,
                 const std::vector<Share>& committee,
                 const NTL::GF2E& beta) {
    std::vector<NTL::GF2E> y_points;
    for (const auto& sh : committee) y_points.push_back(sh.y);
    auto lams = lagrange_coefficients(y_points, beta);
    if (!lams) throw std::runtime_error("committee inadmissible in eval_linear");

    Poly acc;
    for (size_t m = 0; m < committee.size(); m++) {
        Poly scaled;
        long d = NTL::deg(committee[m].sigma);
        for (long j = 0; j <= d; j++) {
            NTL::GF2E c = NTL::coeff(committee[m].sigma, j) * (*lams)[m];
            NTL::SetCoeff(scaled, j, c);
        }
        acc += scaled;
    }
    trunc_mod_xL(acc, cfg.L_sys);
    return acc;
}

std::vector<Share>
cond_share_fragment(const ESSConfig& cfg,
                    const std::vector<uint64_t>& U_next,
                    uint64_t epoch_next,
                    const Poly& fragment,
                    unsigned k_next) {
    ESSConfig next_cfg = cfg;
    next_cfg.k = k_next;
    return cond_share(next_cfg, U_next, epoch_next, fragment);
}

}  
