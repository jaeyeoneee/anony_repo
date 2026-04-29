// experiments/exp7_byzantine.cpp (revised)
//
// Claim C11 revised: The AMD tag (paper Appendix B) detects any tampering
// of a signed message with probability 1 - 2^{-tau}.
//
// NOTE: A full end-to-end Byzantine test requires a TCP transport where
// corrupt messages actually reach the recipient. In this in-process simulation,
// DistJoin computes on locally-stored shares; the Bus is used only for byte
// accounting. Bit-flips on the wire are detected at the AMD-tag micro level
// here. A deployment experiment (future work) will produce end-to-end results
// consistent with this micro-experiment.

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <random>

#include "core/amd_tag.h"
#include "core/field.h"
#include "core/polyring.h"
#include "experiments/exp_common.h"

using namespace epochess;

int main(int argc, char** argv) {
    unsigned s = 16;
    if (argc > 1) s = static_cast<unsigned>(std::atoi(argv[1]));
    init_field(s);

    std::string out_path = "exp7_byzantine.csv";
    if (argc > 2) out_path = argv[2];

    exp::CSVWriter csv(out_path);
    csv.header({"behavior", "tag_bytes", "msg_bytes", "trials", "detections",
                "detection_rate", "theoretical_rate"});

    std::vector<std::string> behaviors = {"NO_TAMPER", "FLIP_1_BIT",
                                          "FLIP_8_BITS", "GARBAGE_FULL"};
    std::vector<size_t> tag_bytes_opts = {4, 8, 16};
    std::vector<size_t> msg_bytes_opts = {64, 256, 1024};
    const int N_TRIALS = 1000;

    auto key = amd_keygen();
    std::mt19937_64 rng(0x7AF51);

    for (const auto& beh : behaviors) {
        for (size_t tb : tag_bytes_opts) {
            for (size_t mb : msg_bytes_opts) {
                int detections = 0;
                for (int t = 0; t < N_TRIALS; t++) {
                    std::vector<uint8_t> msg(mb);
                    for (auto& x : msg) x = static_cast<uint8_t>(rng() & 0xFF);
                    auto tag = amd_tag(key, msg, tb);

                    std::vector<uint8_t> rcv = msg;
                    if (beh == "FLIP_1_BIT") {
                        size_t bit = rng() % (mb * 8);
                        rcv[bit / 8] ^= static_cast<uint8_t>(1u << (bit % 8));
                    } else if (beh == "FLIP_8_BITS") {
                        for (int b = 0; b < 8; b++) {
                            size_t bit = rng() % (mb * 8);
                            rcv[bit / 8] ^= static_cast<uint8_t>(1u << (bit % 8));
                        }
                    } else if (beh == "GARBAGE_FULL") {
                        for (auto& x : rcv) x = static_cast<uint8_t>(rng() & 0xFF);
                    }

                    bool ok = amd_verify(key, rcv, tag);
                    bool tampered = (beh != "NO_TAMPER");
                    if (tampered && !ok) detections++;
                    if (!tampered && ok) detections++;
                }
                double rate = static_cast<double>(detections) / N_TRIALS;
                double theo = (beh == "NO_TAMPER")
                                  ? 1.0
                                  : 1.0 - std::pow(2.0, -static_cast<double>(tb * 8));
                csv.row(beh, tb, mb, N_TRIALS, detections, rate, theo);
                std::cout << "  [exp7] beh=" << beh << " tb=" << tb
                          << " mb=" << mb << " rate=" << rate << std::endl;
            }
        }
    }
    std::cout << "[exp7] wrote " << out_path << std::endl;
    return 0;
}
