#!/usr/bin/env python3
"""
plot_exp8_grinding.py — Visualize grinding-DoS attack effort.
"""
import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

RESULTS = os.environ.get("RESULTS_DIR", "build/results")
OUT = os.environ.get("FIG_DIR", "figures")
os.makedirs(OUT, exist_ok=True)

df = pd.read_csv(f"{RESULTS}/exp8_grinding_dos.csv")
# Aggregate mean trials per (s, k, defense_on).
agg = (df.groupby(["s", "k", "defense_on"])
         .agg(mean_trials=("trials_to_hit", "mean"),
              hit_rate=("found", "mean"))
         .reset_index())

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4.2))

# --- (a) Mean trials-to-collision vs s, per k and defense ---
for k_val in sorted(agg.k.unique()):
    for def_on in [0, 1]:
        sub = agg[(agg.k == k_val) & (agg.defense_on == def_on)].sort_values("s")
        label = f"k={k_val}, " + ("nonce-rotate" if def_on else "no defense")
        ls = "--" if def_on else "-"
        marker = "s" if def_on else "o"
        ax1.plot(sub.s, sub.mean_trials, marker=marker, linestyle=ls,
                 label=label, linewidth=1.4, markersize=6)

# Birthday approximation: q/(k+1) for collisions against k+1 targets.
ss = np.array([8, 12, 16])
for k_val in [5, 10, 20]:
    theo = (2.0 ** ss) / (k_val + 1)
    ax1.plot(ss, theo, ":", color="gray", alpha=0.4, linewidth=1.0)
ax1.plot([], [], ":", color="gray", label=r"theory: $q/(k+1)$")

ax1.set_yscale("log")
ax1.set_xlabel("Extension degree $s$ ($q = 2^s$)")
ax1.set_ylabel("Mean trials to first collision (log)")
ax1.set_title("(a) Grinding-DoS effort: birthday-bound")
ax1.legend(fontsize=8, loc="lower right", ncol=2)
ax1.grid(which="both", alpha=0.3)

# --- (b) Attacker effort normalized to q/(k+1) ---
# Shows that defense does NOT lower the birthday bound — hence commit-reveal
# is required.
ratio_rows = []
for _, row in agg.iterrows():
    q = 2 ** row.s
    expected = q / (row.k + 1)
    ratio_rows.append({
        "s": row.s, "k": row.k, "defense_on": row.defense_on,
        "ratio": row.mean_trials / expected,
    })
ratio_df = pd.DataFrame(ratio_rows)
for k_val in sorted(ratio_df.k.unique()):
    for def_on in [0, 1]:
        sub = ratio_df[(ratio_df.k == k_val) & (ratio_df.defense_on == def_on)].sort_values("s")
        label = f"k={k_val}, " + ("nonce-rotate" if def_on else "no defense")
        ls = "--" if def_on else "-"
        marker = "s" if def_on else "o"
        ax2.plot(sub.s, sub.ratio, marker=marker, linestyle=ls, label=label,
                 linewidth=1.4, markersize=6)

ax2.axhline(1.0, color="k", linestyle=":", alpha=0.5, label="birthday bound")
ax2.set_xlabel("Extension degree $s$")
ax2.set_ylabel(r"observed / expected effort")
ax2.set_title("(b) Nonce rotation does NOT reduce attack effort")
ax2.legend(fontsize=8, loc="upper left", ncol=2)
ax2.grid(alpha=0.3)
ax2.set_ylim(0, 2.0)

plt.tight_layout()
plt.savefig(f"{OUT}/fig_grinding.png", dpi=150)
plt.savefig(f"{OUT}/fig_grinding.pdf")
print(f"wrote {OUT}/fig_grinding.{{png,pdf}}")
print("\nKey numbers for paper writeup:")
for _, row in agg[agg.defense_on == 0].iterrows():
    q = 2 ** row.s
    print(f"  s={row.s}, k={row.k}: mean_trials={row.mean_trials:.0f} "
          f"(theory q/(k+1)={q/(row.k+1):.0f})")
