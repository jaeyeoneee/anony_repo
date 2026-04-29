#!/usr/bin/env python3
"""
plot_wan_amd.py — WAN end-to-end latency (replacing Section 8.4 projections)
                  and AMD tag detection behavior.
"""
import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

RESULTS = os.environ.get("RESULTS_DIR", "build/results")
OUT = os.environ.get("FIG_DIR", "figures")
os.makedirs(OUT, exist_ok=True)

wan = pd.read_csv(f"{RESULTS}/exp5_wan_latency.csv")
amd = pd.read_csv(f"{RESULTS}/exp7_byzantine.csv")

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4.2))

# -------- (a) WAN latency: p50 bars for DistJoin/SoftRefresh at various n, RTT
agg = (wan.groupby(["op", "k", "n", "rtt_ms"])
          .agg(p50=("t_e2e_us", "median"),
               p95=("t_e2e_us", lambda x: np.percentile(x, 95)))
          .reset_index())

# Focus on k=10; group bars by n, colored by RTT.
sub = agg[agg.k == 10]
ops = ["distjoin", "softrefresh"]
rtts = sorted(sub.rtt_ms.unique())
ns = sorted(sub.n.unique())

x = np.arange(len(ns))
width = 0.12
rtt_colors = {0.5: "#a1d99b", 50.0: "#41ab5d", 150.0: "#005a32"}

for op_idx, op in enumerate(ops):
    for i, rtt in enumerate(rtts):
        data = sub[(sub.op == op) & (sub.rtt_ms == rtt)].sort_values("n")
        vals = data.p50.values / 1000.0  # us -> ms
        offset = (op_idx * len(rtts) + i - (len(rtts) * len(ops) - 1) / 2) * width
        hatch = "//" if op == "softrefresh" else ""
        ax1.bar(x + offset, vals, width, label=f"{op} rtt={rtt}ms",
                color=rtt_colors[rtt], edgecolor="k", linewidth=0.5,
                hatch=hatch, alpha=0.9)

ax1.set_xticks(x)
ax1.set_xticklabels([str(n) for n in ns])
ax1.set_xlabel("Active set $n$")
ax1.set_ylabel("E2E latency p50 (ms)")
ax1.set_title("(a) WAN latency (k=10, s=16)")
ax1.legend(fontsize=7, loc="upper left", ncol=2)
ax1.grid(axis="y", alpha=0.3)
ax1.set_yscale("log")

# -------- (b) AMD tag detection rate --------
amd_tamp = amd[amd.behavior != "NO_TAMPER"]
amd_agg = (amd_tamp.groupby(["behavior", "tag_bytes"])
              .agg(rate=("detection_rate", "mean"))
              .reset_index())

pivot = amd_agg.pivot(index="tag_bytes", columns="behavior", values="rate")
pivot.plot.bar(ax=ax2, color=["#fb6a4a", "#ef3b2c", "#99000d"],
               edgecolor="k", linewidth=0.5, alpha=0.9)
ax2.axhline(1.0, color="k", linestyle="--", alpha=0.5)
ax2.set_xlabel("AMD tag size (bytes)")
ax2.set_ylabel("Detection rate")
ax2.set_title("(b) AMD tag detection: 1000 trials/config")
ax2.set_xticks(range(len(pivot)))
ax2.set_xticklabels([f"{tb} ({tb*8}-bit)" for tb in pivot.index], rotation=0)
ax2.set_ylim(0.97, 1.005)
ax2.legend(fontsize=8, loc="lower right")
ax2.grid(axis="y", alpha=0.3)

plt.tight_layout()
plt.savefig(f"{OUT}/fig_wan_amd.png", dpi=150)
plt.savefig(f"{OUT}/fig_wan_amd.pdf")
print(f"wrote {OUT}/fig_wan_amd.{{png,pdf}}")

# Print numeric summary for paper writeup.
print("\n=== WAN latency summary (k=10, median) ===")
for op in ["distjoin", "softrefresh"]:
    for n_val in [30, 100, 300]:
        for rtt in [0.5, 50.0, 150.0]:
            row = agg[(agg.op == op) & (agg.k == 10) & (agg.n == n_val) & (agg.rtt_ms == rtt)]
            if len(row):
                p50_ms = row.iloc[0].p50 / 1000
                p95_ms = row.iloc[0].p95 / 1000
                print(f"  {op} n={n_val} rtt={rtt}ms: p50={p50_ms:.2f}ms p95={p95_ms:.2f}ms")
