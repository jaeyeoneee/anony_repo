# EpochESS: Empirical Validation Artifact

This repository contains the reference C++ implementation and empirical
evaluation artifact for the paper:

> **Upgrading Evolving Secret Sharing: Proactive Refresh and Revocation
> over Polynomial Rings**.

---

## Build & run

### Prerequisites (Ubuntu 24.04)

```bash
apt-get install -y libntl-dev libgmp-dev libsodium-dev libboost-all-dev \
                    cmake g++ python3-pandas python3-matplotlib python3-numpy
```

### Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Run unit tests (should all pass in < 1 s)

```bash
cd build && ctest --output-on-failure
```

Expected output:
```
5/5 Test #5: test_epoch_mgr ................ Passed    0.03 sec
100% tests passed, 0 tests failed out of 5
```

### Run all experiments

```bash
cd build
mkdir -p results
./exp1_distjoin_scaling 16      results/exp1_distjoin_scaling.csv
./exp2_softrefresh_scaling 16   results/exp2_softrefresh_scaling.csv
./exp3_admissibility            results/exp3_admissibility.csv
./exp4_share_size               results/exp4_share_size.csv
./exp5_wan_latency 16           results/exp5_wan_latency.csv
./exp6_packet_loss 16           results/exp6_packet_loss.csv
./exp7_byzantine 16             results/exp7_byzantine.csv
./exp8_grinding_dos             results/exp8_grinding_dos.csv
./exp10_extension_tradeoff      results/exp10_extension_tradeoff.csv
```

Total wall-clock on Intel i7-12700H: ~5 minutes for all 9 experiments.

### Generate figures and summary

```bash
cd ..   # back to repo root
RESULTS_DIR=build/results FIG_DIR=figures python3 tools/plot_scripts/plot_fig4_scaling.py
RESULTS_DIR=build/results FIG_DIR=figures python3 tools/plot_scripts/plot_fig2_tradeoff.py
RESULTS_DIR=build/results FIG_DIR=figures python3 tools/plot_scripts/plot_exp8_grinding.py
RESULTS_DIR=build/results FIG_DIR=figures python3 tools/plot_scripts/plot_wan_amd.py
RESULTS_DIR=build/results FIG_DIR=figures python3 tools/plot_scripts/plot_exp6_packet_loss.py
RESULTS_DIR=build/results FIG_DIR=figures python3 tools/plot_scripts/analyze_all.py
```

Or run everything at once:

```bash
./reproduce.sh
```

---



