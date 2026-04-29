// experiments/exp_common.h — Shared utilities for experiment drivers.

#pragma once

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

namespace epochess {
namespace exp {

using Clock = std::chrono::steady_clock;
using Micros = std::chrono::microseconds;

inline double now_us() {
    auto t = Clock::now().time_since_epoch();
    return std::chrono::duration<double, std::micro>(t).count();
}

// Basic percentile computation on a double vector (sorts internally).
inline double percentile(std::vector<double> xs, double p) {
    if (xs.empty()) return 0.0;
    std::sort(xs.begin(), xs.end());
    double idx = p * (xs.size() - 1);
    size_t lo = static_cast<size_t>(std::floor(idx));
    size_t hi = static_cast<size_t>(std::ceil(idx));
    if (lo == hi) return xs[lo];
    double frac = idx - lo;
    return xs[lo] * (1.0 - frac) + xs[hi] * frac;
}

inline double median(std::vector<double> xs) { return percentile(xs, 0.5); }
inline double p95(std::vector<double> xs) { return percentile(xs, 0.95); }
inline double p99(std::vector<double> xs) { return percentile(xs, 0.99); }

// Write header and row to CSV.
class CSVWriter {
public:
    explicit CSVWriter(const std::string& path) : f_(path) {}
    void header(const std::vector<std::string>& cols) {
        for (size_t i = 0; i < cols.size(); i++) {
            if (i) f_ << ",";
            f_ << cols[i];
        }
        f_ << "\n";
    }
    template <typename... Args>
    void row(Args... args) {
        write_args(args...);
        f_ << "\n";
    }
private:
    std::ofstream f_;
    template <typename T>
    void write_args(T x) { f_ << x; }
    template <typename T, typename... Rest>
    void write_args(T x, Rest... rest) { f_ << x << ","; write_args(rest...); }
};

}  // namespace exp
}  // namespace epochess
