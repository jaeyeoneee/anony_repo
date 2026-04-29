#!/usr/bin/env python3
"""
plot_fig2_tradeoff.py — Reproduces paper Figure 2 (three-way trade-off):
"""
import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

RESULTS = os.environ.get("RESULTS_DIR", "build/results")
OUT = os.environ.get("FIG_DIR", "figures")
os.makedirs(OUT, exist_ok=True)

adm = pd.read_csv(f"{RESULTS}/exp3_admissibility.csv")
ext = pd.read_csv(f"{RESULTS}/exp10_extension_tradeoff.csv")
sz = pd.read_csv(f"{RESULTS}/exp4_share_size.csv")

fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(15, 4.2))

# -------- (a) Admissibility probability --------
colors = {8: "#e41a1c", 12: "#ff7f00", 16: "#4daf4a", 24: "#377eb8", 32: "#984ea3"}
for s_val in [8, 12, 16, 24, 32]:
    sub = adm[adm.s == s_val].sort_values("k")
    # Plot empirical with error bars (95% CI).
    ax1.errorbar(sub.k, sub.p_emp,
                 yerr=np.vstack([sub.p_emp - sub.ci_lo, sub.ci_hi - sub.p_emp]),
                 marker="o", capsize=3, markersize=5, linewidth=1.2,
                 color=colors[s_val], label=f"s={s_val} (emp.)")
    # Overlay theoretical curve as dashed.
    ax1.plot(sub.k, sub.p_theo, "--", color=colors[s_val], alpha=0.5,
             linewidth=1.0)

ax1.axhline(0.95, color="gray", linestyle=":", alpha=0.5)
ax1.text(45, 0.96, "95%", fontsize=8, color="gray")
ax1.set_xlabel("Threshold $k$")
ax1.set_ylabel(r"$p_\mathrm{succ}(k,q)$")
ax1.set_title("(a) Admissibility: empirical (solid) vs theory (dashed)")
ax1.legend(fontsize=8, loc="lower left")
ax1.grid(alpha=0.3)
ax1.set_ylim(-0.02, 1.05)

# -------- (b) n_max vs extension degree --------
# Use the per-s, k=10 row to plot n_max at eps in {1%, 5%, 10%}.
ext_k10 = ext[ext.k == 10].sort_values("s")
ax2.plot(ext_k10.s, ext_k10.n_max_1pct, marker="o", label=r"$\epsilon=1\%$",
         linewidth=1.6)
ax2.plot(ext_k10.s, ext_k10.n_max_5pct, marker="s", label=r"$\epsilon=5\%$",
         linewidth=1.6)
ax2.plot(ext_k10.s, ext_k10.n_max_10pct, marker="^", label=r"$\epsilon=10\%$",
         linewidth=1.6)
ax2.set_yscale("log")
ax2.set_xlabel("Extension degree $s$")
ax2.set_ylabel(r"$n_\mathrm{max}$ (log scale)")
ax2.set_title("(b) Max active set per epoch (k=10)")
ax2.legend(fontsize=9)
ax2.grid(which="both", alpha=0.3)

# -------- (c) Share size vs arrival index i --------
# Use k=10 subset; plot s in {8, 16, 32}.
for s_val in [8, 16, 32]:
    sub = sz[(sz.s == s_val) & (sz.k == 10)].sort_values("i")
    ax3.plot(sub.i, sub.share_bytes_theoretical, marker="o", markersize=4,
             label=f"s={s_val}", linewidth=1.4, color=colors[s_val])
ax3.set_xscale("log")
ax3.set_xlabel("Party arrival index $i$ (log scale)")
ax3.set_ylabel("Share size (bytes)")
ax3.set_title("(c) Share size: $O(\\log i)$ growth (k=10)")
ax3.legend(fontsize=9, loc="upper left")
ax3.grid(which="both", alpha=0.3)

plt.tight_layout()
plt.savefig(f"{OUT}/fig2_tradeoff.png", dpi=150)
plt.savefig(f"{OUT}/fig2_tradeoff.pdf")
print(f"wrote {OUT}/fig2_tradeoff.{{png,pdf}}")
