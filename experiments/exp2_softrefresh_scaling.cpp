// experiments/exp2_softrefresh_scaling.cpp

#include "core/field.h"
#include "core/ess_backend.h"
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

    std::string out_path = "exp2_softrefresh_scaling.csv";
    if (argc > 2) out_path = argv[2];

    exp::CSVWriter csv(out_path);
    csv.header({"s", "k", "n", "trial", "msg_count", "app_bytes",
                "channel_bytes", "latency_us"});

    std::vector<unsigned> ks = {5, 10};
    std::vector<uint64_t> ns = {30, 60, 100, 200, 500};
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

            std::mt19937_64 rng(0xF00D ^ n);
            for (int trial = 0; trial < N_TRIALS; trial++) {
                std::vector<uint64_t> U_next;
                uint64_t revoked = 1 + (rng() % n);
                uint64_t joiner = n + 1 + trial * 10;
                for (uint64_t id : U) if (id != revoked) U_next.push_back(id);
                U_next.push_back(joiner);

                std::vector<Share> overlap;
                for (const auto& sh : shares) {
                    if (sh.identity == revoked) continue;
                    if (overlap.size() < k) overlap.push_back(sh);
                }

                InProcBus bus;
                double t0 = exp::now_us();
                auto r = soft_refresh(cfg, cfg, overlap, U_next, 2, bus);
                double t1 = exp::now_us();
                if (!r.success) continue;

                csv.row(s, k, n, trial, r.msg_count, r.app_bytes,
                        r.channel_bytes, t1 - t0);
            }
            std::cout << "  [exp2] s=" << s << " k=" << k << " n=" << n
                      << " done" << std::endl;
        }
    }
    std::cout << "[exp2] wrote " << out_path << std::endl;
    return 0;
}
