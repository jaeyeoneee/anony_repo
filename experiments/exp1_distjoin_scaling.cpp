// experiments/exp1_distjoin_scaling.cpp


#include "core/field.h"
#include "core/hash_to_field.h"
#include "core/ess_backend.h"
#include "protocol/distjoin.h"
#include "net/inproc_bus.h"
#include "experiments/exp_common.h"

#include <iostream>
#include <random>
#include <algorithm>

using namespace epochess;

int main(int argc, char** argv) {
    unsigned s = 16;
    if (argc > 1) s = static_cast<unsigned>(std::atoi(argv[1]));
    init_field(s);

    std::string out_path = "exp1_distjoin_scaling.csv";
    if (argc > 2) out_path = argv[2];

    exp::CSVWriter csv(out_path);
    csv.header({"s", "k", "n", "trial", "msg_count", "app_bytes",
                "channel_bytes", "latency_us", "expected_msg_count"});

    std::vector<unsigned> ks = {3, 5, 10, 15, 20};
    std::vector<uint64_t> ns = {10, 30, 100, 300, 1000};
    const int N_TRIALS = 50;
    const int N_WARMUP = 5;

    size_t lambda = 128 / s + (128 % s ? 1 : 0);  
    if (lambda < 2) lambda = 2;

    for (unsigned k : ks) {
        size_t max_ell = 20;  
        size_t L_sys = (k - 1) * (max_ell - 1) + lambda;

        ESSConfig cfg{s, lambda, L_sys, k};
        for (uint64_t n : ns) {
            if (n < k + 1) continue;  

            std::vector<uint64_t> U;
            U.reserve(n);
            for (uint64_t i = 1; i <= n; i++) U.push_back(i);
            auto secret = random_secret(cfg.lambda);
            auto shares = cond_share(cfg, U, 1, secret);

            std::mt19937_64 rng(0xE1E1 ^ (uint64_t(k) << 32) ^ n);
            for (int trial = 0; trial < N_TRIALS + N_WARMUP; trial++) {
                std::vector<Share> shuffled = shares;
                std::shuffle(shuffled.begin(), shuffled.end(), rng);
                std::vector<Share> committee(shuffled.begin(),
                                              shuffled.begin() + k + 1);

                uint64_t joiner_id = 0;
                for (uint64_t guess = 10'000'000 + trial * 997;; guess++) {
                    auto y_cand = hash_to_field(guess);
                    bool clash = false;
                    for (const auto& sh : committee) {
                        if (sh.y == y_cand) { clash = true; break; }
                    }
                    if (!clash) { joiner_id = guess; break; }
                }

                InProcBus bus;
                double t0 = exp::now_us();
                auto r = dist_join(cfg, joiner_id, 1, committee, bus);
                double t1 = exp::now_us();

                if (trial < N_WARMUP) continue;  // warmup

                uint64_t expected_msg = static_cast<uint64_t>(k) * (k + 1) + (k + 1);
                csv.row(s, k, n, trial - N_WARMUP,
                        r.msg_count, r.app_bytes, r.channel_bytes,
                        t1 - t0, expected_msg);
            }
            std::cout << "  [exp1] s=" << s << " k=" << k << " n=" << n
                      << " done" << std::endl;
        }
    }
    std::cout << "[exp1] wrote " << out_path << std::endl;
    return 0;
}
