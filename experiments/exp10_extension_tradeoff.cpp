// experiments/exp10_extension_tradeoff.cpp

#include "core/field.h"
#include "core/lagrange.h"
#include "core/ess_backend.h"
#include "experiments/exp_common.h"

#include <cmath>
#include <iostream>

using namespace epochess;

int main(int argc, char** argv) {
    std::string out_path = "exp10_extension_tradeoff.csv";
    if (argc > 1) out_path = argv[1];

    exp::CSVWriter csv(out_path);
    csv.header({"s", "q", "k", "p_succ",
                "n_max_1pct", "n_max_5pct", "n_max_10pct",
                "share_bytes_at_i100", "share_bytes_at_i10k"});

    std::vector<unsigned> ss = {8, 12, 16, 24, 32};
    std::vector<unsigned> ks = {3, 5, 10, 20, 50};

    for (unsigned s : ss) {
        init_field(s);
        uint64_t q = field_size();
        size_t lambda = 128 / s + (128 % s ? 1 : 0);
        if (lambda < 2) lambda = 2;

        auto n_max_for_eps = [q](double eps) {
            return static_cast<uint64_t>(std::floor(std::sqrt(2.0 * q * eps)));
        };

        for (unsigned k : ks) {
            if (k > q) continue;
            double p_succ = theoretical_admissibility(k, q);

            size_t share_i100 = per_identity_bound(100, k, lambda) * field_element_bytes();
            size_t share_i10k = per_identity_bound(10000, k, lambda) * field_element_bytes();

            csv.row(s, q, k, p_succ,
                    n_max_for_eps(0.01), n_max_for_eps(0.05), n_max_for_eps(0.10),
                    share_i100, share_i10k);
        }
        std::cout << "  [exp10] s=" << s << " done" << std::endl;
    }
    std::cout << "[exp10] wrote " << out_path << std::endl;
    return 0;
}
