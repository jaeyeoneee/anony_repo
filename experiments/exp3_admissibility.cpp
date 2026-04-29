// experiments/exp3_admissibility.cpp
//
// Claim C3 (Lemma 5.1): Under the ROM for H, committee admissibility
// probability is p_succ(k, q) = prod_{i=0}^{k-1} (q-i)/q.
//
// Protocol: For each (s, k), draw N_TRIALS committees uniformly at random,
// check distinctness of y-points, count the empirical success rate.
// Report theoretical, empirical, and 95% Wald CI.

#include "core/field.h"
#include "core/lagrange.h"
#include "core/hash_to_field.h"
#include "experiments/exp_common.h"

#include <iostream>
#include <random>
#include <cmath>

using namespace epochess;

int main(int argc, char** argv) {
    std::string out_path = "exp3_admissibility.csv";
    if (argc > 1) out_path = argv[1];

    exp::CSVWriter csv(out_path);
    csv.header({"s", "q", "k", "trials", "success",
                "p_emp", "ci_lo", "ci_hi", "p_theo"});

    std::vector<unsigned> ss = {8, 12, 16, 24, 32};
    std::vector<unsigned> ks = {3, 5, 10, 15, 20, 30, 50};
    const int N_TRIALS = 10000;

    // Use a high-quality RNG to pick identity indices.
    std::mt19937_64 rng(0xADD15);
    std::uniform_int_distribution<uint64_t> id_dist(1, 1'000'000'000);

    for (unsigned s : ss) {
        init_field(s);
        uint64_t q = field_size();
        for (unsigned k : ks) {
            if (k > q) continue;
            int successes = 0;
            for (int t = 0; t < N_TRIALS; t++) {
                // Draw k *random identities*, hash them to F, check distinctness.
                std::vector<NTL::GF2E> ys;
                for (unsigned i = 0; i < k; i++) {
                    uint64_t id = id_dist(rng);
                    ys.push_back(hash_to_field(id));
                }
                if (committee_admissible(ys)) successes++;
            }
            double p_emp = static_cast<double>(successes) / N_TRIALS;
            double se = std::sqrt(p_emp * (1 - p_emp) / N_TRIALS);
            double p_theo = theoretical_admissibility(k, q);
            csv.row(s, q, k, N_TRIALS, successes,
                    p_emp, p_emp - 1.96 * se, p_emp + 1.96 * se, p_theo);
            std::cout << "  [exp3] s=" << s << " k=" << k
                      << " p_emp=" << p_emp
                      << " p_theo=" << p_theo << std::endl;
        }
    }
    std::cout << "[exp3] wrote " << out_path << std::endl;
    return 0;
}
