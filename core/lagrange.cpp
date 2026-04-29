// core/lagrange.cpp

#include "core/lagrange.h"

#include <NTL/GF2E.h>
#include <set>

namespace epochess {

bool committee_admissible(const std::vector<NTL::GF2E>& y_points) {
    size_t k = y_points.size();
    for (size_t i = 0; i < k; i++) {
        for (size_t j = i + 1; j < k; j++) {
            if (y_points[i] == y_points[j]) return false;
        }
    }
    return true;
}

std::optional<std::vector<NTL::GF2E>>
lagrange_coefficients(const std::vector<NTL::GF2E>& y_points,
                      const NTL::GF2E& beta) {
    if (!committee_admissible(y_points)) {
        return std::nullopt;
    }
    size_t k = y_points.size();
    std::vector<NTL::GF2E> lambdas(k);
    for (size_t m = 0; m < k; m++) {
        NTL::GF2E numer, denom;
        NTL::set(numer);   
        NTL::set(denom);   
        for (size_t u = 0; u < k; u++) {
            if (u == m) continue;
            NTL::GF2E diff = beta - y_points[u];
            numer *= diff;
            NTL::GF2E ddiff = y_points[m] - y_points[u];
            denom *= ddiff;
        }
        NTL::GF2E denom_inv;
        NTL::inv(denom_inv, denom);
        lambdas[m] = numer * denom_inv;
    }
    return lambdas;
}

double theoretical_admissibility(unsigned k, uint64_t q) {
    if (k == 0) return 1.0;
    if (q == 0 || k > q) return 0.0;
    double p = 1.0;
    for (unsigned i = 0; i < k; i++) {
        p *= static_cast<double>(q - i) / static_cast<double>(q);
        if (p < 1e-300) return 0.0;
    }
    return p;
}

}  
