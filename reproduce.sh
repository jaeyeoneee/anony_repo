#!/usr/bin/env bash

set -euo pipefail

MODE="${1:-full}"
REPO="$(cd "$(dirname "$0")" && pwd)"
BUILD="$REPO/build"
RESULTS="$BUILD/results"
FIG="$REPO/figures"

echo "=================================================="
echo "EpochESS artifact reproduction ($MODE)"
echo "=================================================="

# --- Build step ---
if [ ! -d "$BUILD" ]; then
    echo ">> Configuring build"
    mkdir -p "$BUILD"
    (cd "$BUILD" && cmake -DCMAKE_BUILD_TYPE=Release .. > /dev/null)
fi

echo ">> Compiling"
(cd "$BUILD" && make -j"$(nproc)" 2>&1 | tail -4)

# --- Unit tests (must pass) ---
echo ">> Running unit tests"
(cd "$BUILD" && ctest --output-on-failure 2>&1 | tail -5)

# --- Experiments ---
mkdir -p "$RESULTS" "$FIG"

S=16
echo ">> Running experiments (s=$S for protocol-sensitive ones)"

run() {
    local name="$1"; shift
    local bin="$1"; shift
    local args=("$@")
    echo "    [$name]"
    "$BUILD/$bin" "${args[@]}" > /dev/null
}

# Fast experiments - always run
run "exp3 admissibility"        exp3_admissibility        "$RESULTS/exp3_admissibility.csv"
run "exp4 share size"           exp4_share_size           "$RESULTS/exp4_share_size.csv"
run "exp10 extension tradeoff"  exp10_extension_tradeoff  "$RESULTS/exp10_extension_tradeoff.csv"
run "exp7 byzantine/AMD"        exp7_byzantine $S         "$RESULTS/exp7_byzantine.csv"

# Medium experiments
run "exp1 distjoin scaling"     exp1_distjoin_scaling $S  "$RESULTS/exp1_distjoin_scaling.csv"
run "exp2 softrefresh scaling"  exp2_softrefresh_scaling $S "$RESULTS/exp2_softrefresh_scaling.csv"
run "exp5 wan latency"          exp5_wan_latency $S       "$RESULTS/exp5_wan_latency.csv"
run "exp6 packet loss"          exp6_packet_loss $S       "$RESULTS/exp6_packet_loss.csv"
run "exp8 grinding DoS"         exp8_grinding_dos         "$RESULTS/exp8_grinding_dos.csv"

# --- Plots ---
echo ">> Generating figures and summary"
cd "$REPO"
export RESULTS_DIR="$RESULTS"
export FIG_DIR="$FIG"
for script in plot_fig2_tradeoff plot_fig4_scaling plot_exp8_grinding \
              plot_exp6_packet_loss plot_wan_amd analyze_all ; do
    echo "    [$script]"
    python3 "tools/plot_scripts/${script}.py" > /dev/null
done

echo ""
echo "=================================================="
echo "Done."
echo "  CSV results: $RESULTS/"
echo "  Figures:     $FIG/*.{png,pdf}"
echo "  Summary:     $FIG/SUMMARY.md"
echo "=================================================="
