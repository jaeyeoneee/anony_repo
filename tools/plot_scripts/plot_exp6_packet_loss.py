#!/usr/bin/env python3
"""
plot_exp6_packet_loss.py — Packet loss robustness.
"""
import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

RESULTS = os.environ.get("RESULTS_DIR", "build/results")
OUT = os.environ.get("FIG_DIR", "figures")
os.makedirs(OUT, exist_ok=True)

df = pd.read_csv(f"{RESULTS}/exp6_packet_loss.csv")
df["completion_rate"] = df.completions / df.trials

fig, ax = plt.subplots(1, 1, figsize=(6.5, 4.2))
for k_val in sorted(df.k.unique()):
    sub = df[df.k == k_val].sort_values("loss_rate")
    ax.plot(sub.loss_rate * 100, sub.completion_rate,
            marker="o", linewidth=1.6, markersize=7, label=f"k={k_val} (measured)")

# Overlay theoretical (1-p)^M curve. For DistJoin, M = k(k+1) + (k+1).
xs = np.linspace(0, 0.12, 100)
for k_val in sorted(df.k.unique()):
    M = k_val * (k_val + 1) + (k_val + 1)
    ys = (1 - xs) ** M
    ax.plot(xs * 100, ys, "--", alpha=0.45, linewidth=1.0,
            label=f"k={k_val} theory: $(1-p)^{{{M}}}$")

ax.set_xlabel("Packet loss rate (%)")
ax.set_ylabel("DistJoin completion rate")
ax.set_title("DistJoin robustness to packet loss (no retransmission)")
ax.legend(fontsize=8, loc="lower left")
ax.grid(alpha=0.3)
ax.set_ylim(-0.05, 1.05)

plt.tight_layout()
plt.savefig(f"{OUT}/fig_packet_loss.png", dpi=150)
plt.savefig(f"{OUT}/fig_packet_loss.pdf")
print(f"wrote {OUT}/fig_packet_loss.{{png,pdf}}")
print()
print("Expected messages (M) per DistJoin:")
for k_val in sorted(df.k.unique()):
    M = k_val * (k_val + 1) + (k_val + 1)
    print(f"  k={k_val}: M={M}; at p=5%, (1-p)^M = {(1-0.05)**M:.4f}")
