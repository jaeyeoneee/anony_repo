// experiments/exp5_wan_latency.cpp

#include <algorithm>
#include "core/field.h"
#include "core/hash_to_field.h"
#include "core/ess_backend.h"
#include "protocol/distjoin.h"
#include "protocol/softrefresh.h"
#include "net/inproc_bus.h"
#include "experiments/exp_common.h"

#include <iostream>
#include <random>

using namespace epochess;

int main(int argc, char** argv) {
    unsigned s = 16;
    if (argc > 1) s = static_cast<unsigned>(std::atoi(argv[1]));
    init_field(s);

    std::string out_path = "exp5_wan_latency.csv";
    if (argc > 2) out_path = argv[2];

    exp::CSVWriter csv(out_path);
    csv.header({"op", "s", "k", "n", "rtt_ms", "trial",
                "t_crypto_us", "t_e2e_us"});

    std::vector<unsigned> ks = {5, 10};
    std::vector<uint64_t> ns = {30, 100, 300};
    std::vector<double> rtts_ms = {0.5, 50.0, 150.0};
    const int N_TRIALS = 20;

    size_t lambda = 128 / s + (128 % s ? 1 : 0);
    if (lambda < 2) lambda = 2;

    for (unsigned k : ks) {
        size_t max_ell = 20;
        size_t L_sys = (k - 1) * (max_ell - 1) + lambda;
        ESSConfig cfg{s, lambda, L_sys, k};

        for (uint64_t n : ns) {
            if (n < k + 1) continue;

            std::vector<uint64_t> U;
            for (uint64_t i = 1; i <= n; i++) U.push_back(i);
            auto secret = random_secret(cfg.lambda);
            auto shares = cond_share(cfg, U, 1, secret);

            std::mt19937_64 rng(0xA5A5 ^ n ^ k);

            for (int t = 0; t < N_TRIALS; t++) {
                std::vector<Share> shuffled = shares;
                std::shuffle(shuffled.begin(), shuffled.end(), rng);
                std::vector<Share> committee(shuffled.begin(),
                                              shuffled.begin() + k + 1);
                uint64_t joiner = 100'000'000 + t * 31;
                while (true) {
                    auto y = hash_to_field(joiner);
                    bool clash = false;
                    for (const auto& sh : committee) if (sh.y == y) { clash = true; break; }
                    if (!clash) break;
                    joiner++;
                }
                InProcBus bus;
                double t0 = exp::now_us();
                auto r = dist_join(cfg, joiner, 1, committee, bus);
                double t1 = exp::now_us();
                if (!r.success) continue;
                double t_crypto = t1 - t0;
                for (double rtt : rtts_ms) {
                    double e2e = t_crypto + 2.0 * rtt * 1000.0;  // us
                    csv.row("distjoin", s, k, n, rtt, t, t_crypto, e2e);
                }
            }

            for (int t = 0; t < N_TRIALS; t++) {
                std::vector<uint64_t> U_next = U;
                U_next.push_back(n + 1 + t);
                std::vector<Share> overlap(shares.begin(),
                                            shares.begin() + k);

                InProcBus bus;
                double t0 = exp::now_us();
                auto r = soft_refresh(cfg, cfg, overlap, U_next, 2, bus);
                double t1 = exp::now_us();
                if (!r.success) continue;
                double t_crypto = t1 - t0;
                for (double rtt : rtts_ms) {
                    double e2e = t_crypto + 1.0 * rtt * 1000.0;
                    csv.row("softrefresh", s, k, n, rtt, t, t_crypto, e2e);
                }
            }
            std::cout << "  [exp5] s=" << s << " k=" << k << " n=" << n
                      << " done" << std::endl;
        }
    }
    std::cout << "[exp5] wrote " << out_path << std::endl;
    return 0;
}
