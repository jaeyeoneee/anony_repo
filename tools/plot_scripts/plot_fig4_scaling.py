#!/usr/bin/env python3
"""
plot_fig4_scaling.py — Reproduces paper Figure 4:
  (left)  DistJoin scales as O(k^2), independent of n
  (right) SoftRefresh scales linearly with n

Input:  results/exp1_distjoin_scaling.csv
        results/exp2_softrefresh_scaling.csv
Output: figures/fig4_scaling.png, figures/fig4_scaling.pdf

The key story this figure tells, backed by measured bytes and messages:
  - DistJoin's message count is EXACTLY k(k+1)+(k+1), independent of n
  - SoftRefresh's message count is EXACTLY k * n_next (linear)
"""
import os
import sys
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

RESULTS = os.environ.get("RESULTS_DIR", "build/results")
OUT = os.environ.get("FIG_DIR", "figures")
os.makedirs(OUT, exist_ok=True)

# ------------------------------ DistJoin: k-scaling, n-independence ---------
dj = pd.read_csv(f"{RESULTS}/exp1_distjoin_scaling.csv")
sr = pd.read_csv(f"{RESULTS}/exp2_softrefresh_scaling.csv")

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4.2))

# --- Left: DistJoin message count vs k, multiple n overlaid ---
dj_agg = (dj.groupby(["s", "k", "n"])
            .agg(msg_mean=("msg_count", "mean"),
                 msg_std=("msg_count", "std"),
                 bytes_mean=("app_bytes", "mean"),
                 latency_us_p50=("latency_us", "median"))
            .reset_index())

s_focus = 16
dj_fs = dj_agg[dj_agg.s == s_focus]
markers = {10: "o", 30: "s", 100: "^", 300: "D", 1000: "v"}
for n_val in sorted(dj_fs.n.unique()):
    sub = dj_fs[dj_fs.n == n_val].sort_values("k")
    ax1.plot(sub.k, sub.msg_mean, marker=markers.get(n_val, "x"),
             label=f"n={n_val}", linewidth=1.6, markersize=7)

# Theoretical O(k^2) curve: k(k+1) + (k+1) = (k+1)^2
ks = np.array(sorted(dj_fs.k.unique()))
theo = (ks + 1) ** 2
ax1.plot(ks, theo, "k--", linewidth=1.2, alpha=0.6,
         label=r"theory: $(k+1)^2$")

ax1.set_xlabel("Threshold $k$")
ax1.set_ylabel("DistJoin messages (median)")
ax1.set_title(f"DistJoin: $O(k^2)$, independent of $n$  ($s={s_focus}$)")
ax1.legend(fontsize=9, loc="upper left")
ax1.grid(alpha=0.3)

# --- Right: SoftRefresh bytes vs n, multiple k ---
sr_agg = (sr.groupby(["s", "k", "n"])
            .agg(msg_mean=("msg_count", "mean"),
                 bytes_mean=("app_bytes", "mean"))
            .reset_index())
sr_fs = sr_agg[sr_agg.s == s_focus].sort_values(["k", "n"])
for k_val in sorted(sr_fs.k.unique()):
    sub = sr_fs[sr_fs.k == k_val]
    ax2.plot(sub.n, sub.bytes_mean / 1024, marker="o",
             label=f"k={k_val}", linewidth=1.6, markersize=7)

ax2.set_xlabel("Active set size $n_{e+1}$")
ax2.set_ylabel("SoftRefresh bytes (KB)")
ax2.set_title("SoftRefresh: linear in $n$")
ax2.legend(fontsize=9, loc="upper left")
ax2.grid(alpha=0.3)

# Print linear-fit R^2 for each k (sanity check for C2 claim).
print("SoftRefresh linear fit (bytes ~ alpha*n + beta):")
for k_val in sorted(sr_fs.k.unique()):
    sub = sr_fs[sr_fs.k == k_val]
    slope, intercept = np.polyfit(sub.n.values, sub.bytes_mean.values, 1)
    pred = slope * sub.n.values + intercept
    ss_res = np.sum((sub.bytes_mean.values - pred) ** 2)
    ss_tot = np.sum((sub.bytes_mean.values - sub.bytes_mean.mean()) ** 2)
    r2 = 1 - ss_res / ss_tot if ss_tot > 0 else 0
    print(f"  k={k_val}: slope={slope:.2f} B/node, R^2={r2:.6f}")

plt.tight_layout()
plt.savefig(f"{OUT}/fig4_scaling.png", dpi=150)
plt.savefig(f"{OUT}/fig4_scaling.pdf")
print(f"wrote {OUT}/fig4_scaling.{{png,pdf}}")
