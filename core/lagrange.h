// core/lagrange.h — Pointwise Lagrange interpolation over F_{2^s}.

#pragma once

#include "core/field.h"

#include <vector>
#include <optional>

namespace epochess {

// Check that all evaluation points are distinct.
bool committee_admissible(const std::vector<NTL::GF2E>& y_points);

// Compute Lagrange coefficients {lambda_m(beta; T)}_{m=1}^{k}.
std::optional<std::vector<NTL::GF2E>>
lagrange_coefficients(const std::vector<NTL::GF2E>& y_points,
                      const NTL::GF2E& beta);

// Theoretical admissibility probability:
//   p_succ(k, q) = prod_{i=0}^{k-1} (q-i)/q.
double theoretical_admissibility(unsigned k, uint64_t q);

} 
