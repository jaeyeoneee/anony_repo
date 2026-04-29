// core/polyring.h — Polynomial ring F_{2^s}[x]/<x^L>

#pragma once

#include <NTL/GF2EX.h>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace epochess {

using Poly = NTL::GF2EX;

// Truncate a polynomial in-place to degree < L (i.e., reduce mod x^L).
void trunc_mod_xL(Poly& f, size_t L);

// Compute g = (a * b) mod x^L. 
Poly mul_mod_xL(const Poly& a, const Poly& b, size_t L);

// Sample a uniformly random polynomial of degree < L in F_{2^s}[x]/<x^L>.
Poly random_poly(size_t L);

// Evaluate polynomial f(y) at y in F_{2^s}. The value is a field element.
NTL::GF2E eval_at(const Poly& f, const NTL::GF2E& y);

// Project f in F_{2^s}[x]/<x^L> to F_{2^s}[x]/<x^lambda> by truncation.
Poly project(const Poly& f, size_t lambda);

// Serialize/deserialize a polynomial of maximum length L (padded).
std::vector<uint8_t> serialize(const Poly& f, size_t L);
Poly deserialize(const uint8_t* data, size_t len, size_t L);

// Size in bytes of a serialized polynomial of length L.
size_t serialized_bytes(size_t L);

} 
