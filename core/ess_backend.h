// core/ess_backend.h — Evolving k-threshold secret sharing backend over
// F_{2^s}[x]/<x^L>.

#pragma once

#include "core/field.h"
#include "core/polyring.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace epochess {

// Configuration for an ESS instantiation.
struct ESSConfig {
    unsigned s;          // extension degree (8, 12, 16, 24, 32)
    size_t lambda;       // secret degree bound: S in F_{2^s}[x]/<x^lambda>
    size_t L_sys;        // system ring: F_{2^s}[x]/<x^{L_sys}>
    unsigned k;          // threshold
};

// Codeword length ell_i = 1 + floor(log2 i) for identity index i >= 1.
inline size_t codeword_length(uint64_t i) {
    if (i <= 1) return 1;
    size_t len = 0;
    uint64_t v = i;
    while (v > 0) { len++; v >>= 1; }
    return len;
}

// Per-identity compact bound: L_i = (k-1) * (ell_i - 1) + lambda.
inline size_t per_identity_bound(uint64_t i, unsigned k, size_t lambda) {
    size_t ell = codeword_length(i);
    size_t ell_minus_1 = (ell >= 1) ? (ell - 1) : 0;
    return (static_cast<size_t>(k) - 1) * ell_minus_1 + lambda;
}

// A share held by party i: polynomial + identity + epoch tag.
struct Share {
    uint64_t identity;        // party index i
    uint64_t epoch;           // epoch e
    NTL::GF2E y;              // evaluation point y_i = H(i)
    Poly sigma;               // share polynomial in F_{2^s}[x]/<x^{L_i}>
    size_t L_i;               // per-identity compact bound
};

// Sample a secret: a uniform random polynomial in F_{2^s}[x]/<x^lambda>.
Poly random_secret(size_t lambda);

// CondShare(U, k, s): generate shares for identities in U, under secret s.
std::vector<Share>
cond_share(const ESSConfig& cfg,
           const std::vector<uint64_t>& U,
           uint64_t epoch,
           const Poly& secret);

// Reconstruct the secret from k shares.
Poly reconstruct(const ESSConfig& cfg, const std::vector<Share>& shares);

// Public linear evaluation: compute sum_{i in T} lambda_i(beta; T) * sigma_i
// over F_{2^s}[x]/<x^{L_sys}>. This is Eval(beta; T, {sigma_i}).
Poly eval_linear(const ESSConfig& cfg,
                 const std::vector<Share>& committee,
                 const NTL::GF2E& beta);

// Sample a fresh sharing of the same secret for a given target set.
std::vector<Share>
cond_share_fragment(const ESSConfig& cfg,
                    const std::vector<uint64_t>& U_next,
                    uint64_t epoch_next,
                    const Poly& fragment,
                    unsigned k_next);

}
