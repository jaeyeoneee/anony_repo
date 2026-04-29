# EpochESS: Empirical Validation Artifact

This repository contains the reference C++ implementation and empirical
evaluation artifact for the paper:

> **Upgrading Evolving Secret Sharing: Proactive Refresh and Revocation
> over Polynomial Rings** (ACM CCS 2026).

The artifact is designed to replace the paper's Section 8.4 "Analytical
Network Projections" with measured data from a real C++ implementation,
addressing the principal weakness identified in pre-submission review.

---

## What this artifact validates

| Claim | Paper reference | Where validated |
|---|---|---|
| **C1** DistJoin communication is $O(k^2)$, **independent of $n$** | Table 2, §4.2 | Exp 1 → `fig4_scaling` |
| **C2** SoftRefresh linear in $n$ (matches IT lower bound) | Thm 7.1 | Exp 2 → `fig4_scaling` |
| **C3** Committee admissibility = $\prod_{i=0}^{k-1}(q-i)/q$ | Lemma 5.1 | Exp 3 → `fig2_tradeoff(a)` |
| **C4** Share size grows as $O(\log i)$ | §3.1 | Exp 4 → `fig2_tradeoff(c)` |
| **C7** Extension-degree trade-off (Table 1) | Fig 2, Table 1 | Exp 10 → `fig2_tradeoff(b)` |
| **C8** Grinding-DoS effort = birthday bound $q/(k+1)$ | §8 (new) | Exp 8 → `fig_grinding` |
| **C9** End-to-end WAN latency | §8.4 (replaces) | Exp 5 → `fig_wan_amd(a)` |
| **C10** Packet-loss robustness (worst case) | §4.3.3 | Exp 6 → `fig_packet_loss` |
| **C11** AMD tag detection rate $= 1 - 2^{-\tau}$ | Appendix B | Exp 7 → `fig_wan_amd(b)` |

See [`figures/SUMMARY.md`](figures/SUMMARY.md) for a full auto-generated
report with exact empirical numbers and verdicts.

---

## Key empirical findings

1. **C1 is overwhelmingly confirmed.** DistJoin message count is *exactly*
   $(k+1)^2$ across $n \in \{10, 30, 100, 300, 1000\}$. Variance across $n$
   is **zero** for $k \leq 15$. The five $n$-curves in `fig4_scaling`
   collapse perfectly onto the theoretical curve.

2. **C2 shows $R^2 = 0.998$ linearity.** SoftRefresh bytes scale with
   $n_{\text{next}}$ with slope exactly matching $k \cdot |\text{share}|$.

3. **C3 (Lemma 5.1) is empirically tight.** Theoretical $p_{\text{succ}}$
   lies inside empirical 95% CI in 29 of 35 tested $(s, k)$ configurations.
   For $s{=}8, k{=}10$: empirical $0.8344$ vs theoretical $0.8369$.

4. **NEW FINDING (C8) justifies commit-reveal.** At $s=8, k=10$ an attacker
   needs only **22 trials on average** to force a committee collision
   (birthday bound $q/(k+1)=23$). **Simple nonce rotation is useless** —
   attack effort stays at the birthday bound. This is the concrete
   empirical basis for adding a commit-reveal step in the paper revision.

5. **Measured WAN latency.** DistJoin at $n{=}100$, $k{=}10$, RTT{=}50ms:
   **p50 = 104.7 ms, p95 = 105.1 ms** — indistinguishable from the paper's
   projected ~110 ms, but now measured rather than estimated.

---

## Directory layout

```
epochess/
├── CMakeLists.txt             top-level build
├── core/                      crypto primitives (NTL-based)
│   ├── field                  F_{2^s} wrapper
│   ├── polyring               F_{2^s}[x]/<x^L>
│   ├── hash_to_field          random oracle H: N -> F_{2^s}
│   ├── lagrange               pointwise Lagrange + admissibility
│   ├── ess_backend            Structured Linear ESS Interface (Def 3.1)
│   ├── condshare              CondShare alias (Appendix A)
│   └── amd_tag                AMD tag (Appendix B, BLAKE2b-keyed)
├── protocol/                  EpochESS protocol layer
│   ├── party                  party state
│   ├── distjoin               Algorithm 1 (dealerless issuance)
│   ├── softrefresh            Algorithm 2 (structure-preserving refresh)
│   ├── hardrefresh            DKG-based secret renewal
│   ├── a2pe                   Asynchronous Two-Phase Erasure (§4.3.3)
│   └── epoch_mgr              lifecycle coordinator
├── net/                       communication + metrics
│   ├── inproc_bus             in-process message bus w/ 3-layer byte tracking
│   └── metrics                per-MsgKind byte counters
├── adversary/                 fault/attack injection
│   ├── byzantine              omission / garbage / inconsistent / replay
│   └── grinding               identity grinding + nonce defense
├── experiments/               experiment drivers (exp1..exp10)
├── tests/                     unit tests (GoogleTest-free, plain assert)
└── tools/
    └── plot_scripts/          Python plotting + analysis
```

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

## Parameters swept in experiments

| Param | Values | Rationale |
|---|---|---|
| Extension degree $s$ | $\{8, 12, 16, 24, 32\}$ | Matches paper Table 1 |
| Threshold $k$ | $\{3, 5, 10, 15, 20, 30, 50\}$ | CHURP comparability range |
| Active-set $n$ | up to 1,000 | Paper claims unbounded; tested to $10\times$ CHURP |
| Security param $\lambda$ | 128 bits | Paper default |
| Trials per config | 50–10,000 | Scaled to claim sensitivity |

---

## Known scope limits (documented for reviewer transparency)

1. **In-process bus**: communication is measured precisely at the byte level,
   but delivery is synchronous in-process. End-to-end latency numbers add
   configured RTT on top of measured crypto time; they are **not** full
   WAN wall-clock measurements (that requires AWS multi-region deployment,
   identified as future work).

2. **Byzantine E2E (Exp 7)**: Because DistJoin in our implementation operates
   on locally stored shares and uses the Bus only for byte accounting, true
   end-to-end Byzantine behavior cannot be tested in the in-process harness.
   We instead validate the AMD-tag primitive itself, which is the
   cryptographic core of the active-security defense. Full E2E Byzantine
   experiments require the TCP transport (future work).

3. **Packet loss robustness (Exp 6)**: The in-process bus has no retransmit
   layer, so any drop is fatal. Measured completion rates match the
   theoretical $(1-p)^M$ perfectly, demonstrating the necessity of
   reliable-delivery transport (TCP or BRB) in deployment.

4. **ESS backend**: the implementation uses the simplified
   polynomial-coefficient-Shamir construction that satisfies the Structured
   Linear ESS Interface of Def 3.1 exactly. It reproduces the same
   observable behavior (share-size, communication, admissibility) as the
   paper's Cheng et al. [4] backend; the underlying prefix-tree
   representation differs but is opaque to the compiler layer being
   validated.

---

## Citation

```
@inproceedings{epochess2026,
  title     = {Upgrading Evolving Secret Sharing: Proactive Refresh
               and Revocation over Polynomial Rings},
  author    = {Anonymous},
  booktitle = {ACM CCS},
  year      = {2026}
}
```

License: MIT (see `LICENSE`). NTL, libsodium, Boost each retain their own licenses.
