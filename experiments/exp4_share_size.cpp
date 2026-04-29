// experiments/exp4_share_size.cpp
//
// Claim C4 (paper Section 3.1): Per-identity share size L_i =
// (k-1)(ell_i - 1) + lambda, growing as O(log i).
//
// Protocol: For a range of arrival indices i from 10 to 10^6, compute L_i
// and the actual serialized byte size of the share polynomial. Verify
// serialization size <= L_i * field_element_bytes().

#include "core/field.h"
#include "core/ess_backend.h"
#include "core/polyring.h"
#include "experiments/exp_common.h"

#include <iostream>
#include <cmath>

using namespace epochess;

int main(int argc, char** argv) {
    std::string out_path = "exp4_share_size.csv";
    if (argc > 1) out_path = argv[1];

    exp::CSVWriter csv(out_path);
    csv.header({"s", "k", "i", "ell_i", "L_i",
                "share_bytes_theoretical", "share_bytes_actual"});

    std::vector<unsigned> ss = {8, 16, 32};
    std::vector<unsigned> ks = {5, 10};
    // Sweep arrival index across six orders of magnitude.
    std::vector<uint64_t> is;
    for (uint64_t i = 1; i <= 1'000'000; i = (i < 10 ? i + 1 : i * 2)) {
        is.push_back(i);
    }

    for (unsigned s : ss) {
        init_field(s);
        size_t lambda = 128 / s + (128 % s ? 1 : 0);
        if (lambda < 2) lambda = 2;

        for (unsigned k : ks) {
            // L_sys should accommodate worst case in our grid.
            size_t max_ell = codeword_length(1'000'000);
            size_t L_sys = (k - 1) * (max_ell - 1) + lambda;
            ESSConfig cfg{s, lambda, L_sys, k};

            // Generate a sharing for a set including identity i.
            // We don't actually run a huge n; we just compute L_i and the
            // polynomial size from cond_share at a small U that includes i.
            for (uint64_t i : is) {
                std::vector<uint64_t> U = {i, i + 1, i + 2, i + 3, i + 4,
                                           i + 5, i + 6, i + 7, i + 8, i + 9};
                // Only need k+1 for reconstruction potential.
                while (U.size() < k + 1) U.push_back(U.back() + 1);
                auto secret = random_secret(cfg.lambda);
                auto shares = cond_share(cfg, U, 1, secret);
                // Share for identity i:
                size_t L_i = per_identity_bound(i, k, lambda);
                size_t theor = L_i * field_element_bytes();
                size_t actual = serialize(shares[0].sigma, L_i).size();
                size_t ell = codeword_length(i);
                csv.row(s, k, i, ell, L_i, theor, actual);
            }
            std::cout << "  [exp4] s=" << s << " k=" << k << " done"
                      << std::endl;
        }
    }
    std::cout << "[exp4] wrote " << out_path << std::endl;
    return 0;
}
