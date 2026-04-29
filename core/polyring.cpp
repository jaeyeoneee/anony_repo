// core/polyring.cpp — Polynomial ring implementation.

#include "core/polyring.h"
#include "core/field.h"

#include <NTL/GF2EX.h>
#include <NTL/vec_GF2E.h>
#include <stdexcept>

namespace epochess {

void trunc_mod_xL(Poly& f, size_t L) {
    if (static_cast<size_t>(NTL::deg(f) + 1) > L) {
        NTL::trunc(f, f, static_cast<long>(L));
    }
}

Poly mul_mod_xL(const Poly& a, const Poly& b, size_t L) {
    Poly c;
    NTL::MulTrunc(c, a, b, static_cast<long>(L));
    return c;
}

Poly random_poly(size_t L) {
    Poly f;
    for (size_t i = 0; i < L; i++) {
        NTL::GF2E c;
        NTL::random(c);
        NTL::SetCoeff(f, static_cast<long>(i), c);
    }
    return f;
}

NTL::GF2E eval_at(const Poly& f, const NTL::GF2E& y) {
    NTL::GF2E out;
    NTL::eval(out, f, y);
    return out;
}

Poly project(const Poly& f, size_t lambda) {
    Poly g = f;
    trunc_mod_xL(g, lambda);
    return g;
}

size_t serialized_bytes(size_t L) {
    return L * field_element_bytes();
}

std::vector<uint8_t> serialize(const Poly& f, size_t L) {
    size_t elem_bytes = field_element_bytes();
    std::vector<uint8_t> out(L * elem_bytes, 0);
    long d = NTL::deg(f);
    for (size_t i = 0; i < L; i++) {
        NTL::GF2E c;
        if (static_cast<long>(i) <= d) {
            c = NTL::coeff(f, static_cast<long>(i));
        } else {
            NTL::clear(c);
        }
        auto ser = epochess::serialize(c);
        for (size_t j = 0; j < elem_bytes; j++) {
            out[i * elem_bytes + j] = ser[j];
        }
    }
    return out;
}

Poly deserialize(const uint8_t* data, size_t len, size_t L) {
    size_t elem_bytes = field_element_bytes();
    if (len != L * elem_bytes) {
        throw std::invalid_argument("polynomial serialization length mismatch");
    }
    Poly f;
    for (size_t i = 0; i < L; i++) {
        NTL::GF2E c = epochess::deserialize(data + i * elem_bytes, elem_bytes);
        NTL::SetCoeff(f, static_cast<long>(i), c);
    }
    return f;
}

}  