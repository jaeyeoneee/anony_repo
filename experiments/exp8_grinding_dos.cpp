// experiments/exp8_grinding_dos.cpp

#include "core/field.h"
#include "core/ess_backend.h"
#include "core/hash_to_field.h"
#include "adversary/grinding.h"
#include "experiments/exp_common.h"

#include <iostream>
#include <random>
#include <set>

using namespace epochess;

int main(int argc, char** argv) {
    std::string out_path = "exp8_grinding_dos.csv";
    if (argc > 1) out_path = argv[1];

    exp::CSVWriter csv(out_path);
    csv.header({"s", "k", "defense_on", "budget",
                "trial", "found", "trials_to_hit"});

    std::vector<unsigned> ss = {8, 12, 16};
    std::vector<unsigned> ks = {5, 10, 20};
    const int N_TRIALS = 100;

    for (unsigned s : ss) {
        init_field(s);
        uint64_t q = field_size();
        // Budget scaled to field size: attacker allowed 100q trials,
        // which ensures collision almost always found for small fields
        // without defense.
        uint64_t budget = q * 100ull;
        if (budget > 10'000'000) budget = 10'000'000;

        for (unsigned k : ks) {
            if (k + 1 > q) continue;
            for (bool defense : {false, true}) {
                std::mt19937_64 rng(0x6E9 ^ s ^ k ^ (defense ? 1 : 0));
                for (int t = 0; t < N_TRIALS; t++) {
                    // Build a random committee of k+1 "real" identities.
                    std::vector<NTL::GF2E> committee_y;
                    std::set<uint64_t> used;
                    while (committee_y.size() < k + 1) {
                        uint64_t cid = 1 + (rng() % 1'000'000);
                        if (used.count(cid)) continue;
                        used.insert(cid);
                        committee_y.push_back(hash_to_field(cid));
                    }
                    uint64_t nonce = defense ? rng() : 0ull;
                    auto stats = grind_count(committee_y, budget, nonce);
                    csv.row(s, k, defense ? 1 : 0, budget,
                            t, stats.found ? 1 : 0, stats.trials);
                }
                std::cout << "  [exp8] s=" << s << " k=" << k
                          << " def=" << defense << " done" << std::endl;
            }
        }
    }
    std::cout << "[exp8] wrote " << out_path << std::endl;
    return 0;
}
