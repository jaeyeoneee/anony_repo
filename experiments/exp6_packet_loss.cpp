// experiments/exp6_packet_loss.cpp

#include <algorithm>
#include "core/field.h"
#include "core/hash_to_field.h"
#include "core/ess_backend.h"
#include "protocol/distjoin.h"
#include "net/inproc_bus.h"
#include "experiments/exp_common.h"

#include <iostream>
#include <random>

using namespace epochess;

int main(int argc, char** argv) {
    unsigned s = 16;
    if (argc > 1) s = static_cast<unsigned>(std::atoi(argv[1]));
    init_field(s);

    std::string out_path = "exp6_packet_loss.csv";
    if (argc > 2) out_path = argv[2];

    exp::CSVWriter csv(out_path);
    csv.header({"op", "s", "k", "n", "loss_rate",
                "trials", "completions",
                "mean_dropped_msgs", "mean_app_bytes"});

    std::vector<unsigned> ks = {5, 10};
    std::vector<uint64_t> ns = {100};
    std::vector<double> loss_rates = {0.0, 0.01, 0.05, 0.10};
    const int N_TRIALS = 200;

    size_t lambda = 128 / s + (128 % s ? 1 : 0);
    if (lambda < 2) lambda = 2;

    for (unsigned k : ks) {
        size_t L_sys = (k - 1) * 20 + lambda;
        ESSConfig cfg{s, lambda, L_sys, k};

        for (uint64_t n : ns) {
            if (n < k + 1) continue;

            std::vector<uint64_t> U;
            for (uint64_t i = 1; i <= n; i++) U.push_back(i);
            auto secret = random_secret(cfg.lambda);
            auto shares = cond_share(cfg, U, 1, secret);

            for (double lr : loss_rates) {
                int completions = 0;
                double sum_dropped = 0;
                double sum_bytes = 0;
                std::mt19937_64 rng(0xD1E ^ n ^ static_cast<uint64_t>(lr * 1e6));

                for (int t = 0; t < N_TRIALS; t++) {
                    std::vector<Share> shuffled = shares;
                    std::shuffle(shuffled.begin(), shuffled.end(), rng);
                    std::vector<Share> committee(shuffled.begin(),
                                                   shuffled.begin() + k + 1);
                    uint64_t joiner = 700'000'000 + t * 31;
                    while (true) {
                        auto y = hash_to_field(joiner);
                        bool clash = false;
                        for (const auto& sh : committee) if (sh.y == y) { clash = true; break; }
                        if (!clash) break;
                        joiner++;
                    }
                    InProcBus bus;
                    bus.set_rng_seed(rng());
                    bus.set_loss_rate(lr);

                    auto r = dist_join(cfg, joiner, 1, committee, bus);
                    uint64_t dropped = bus.metrics().dropped_count;
                    bool ok = r.success && (dropped == 0);
                    if (ok) completions++;
                    sum_dropped += dropped;
                    sum_bytes += r.app_bytes;
                }
                csv.row("distjoin", s, k, n, lr, N_TRIALS, completions,
                        sum_dropped / N_TRIALS, sum_bytes / N_TRIALS);
            }
            std::cout << "  [exp6] s=" << s << " k=" << k << " n=" << n
                      << " done" << std::endl;
        }
    }
    std::cout << "[exp6] wrote " << out_path << std::endl;
    return 0;
}
