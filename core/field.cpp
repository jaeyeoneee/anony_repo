// core/field.cpp — F_{2^s} implementation.

#include "core/field.h"

#include <NTL/GF2XFactoring.h>
#include <stdexcept>
#include <mutex>
#include <thread>

namespace epochess {

namespace {
thread_local unsigned g_s = 0;
} 

void init_field(unsigned s) {
    if (s == 0 || s > 63) {
        throw std::invalid_argument("field degree must be in [1, 63]");
    }
    NTL::GF2X f;
    NTL::BuildIrred(f, s);
    NTL::GF2E::init(f);
    g_s = s;
}

unsigned field_degree() {
    if (g_s == 0) throw std::runtime_error("field not initialized");
    return g_s;
}

uint64_t field_size() {
    return uint64_t{1} << field_degree();
}

size_t field_element_bytes() {
    return (field_degree() + 7) / 8;
}

FE random_nonzero_element() {
    FE x;
    do {
        NTL::random(x);
    } while (NTL::IsZero(x));
    return x;
}

FE random_element() {
    FE x;
    NTL::random(x);
    return x;
}

FE from_uint(uint64_t v) {
    unsigned s = field_degree();
    if (s < 64 && v >= (uint64_t{1} << s)) {
        throw std::invalid_argument("value out of range for field");
    }
    NTL::GF2X poly;
    for (unsigned i = 0; i < s; i++) {
        if ((v >> i) & 1) {
            NTL::SetCoeff(poly, i, 1);
        }
    }
    FE out;
    NTL::conv(out, poly);
    return out;
}

uint64_t to_uint(const FE& x) {
    unsigned s = field_degree();
    NTL::GF2X poly = NTL::conv<NTL::GF2X>(x);
    uint64_t v = 0;
    for (unsigned i = 0; i < s; i++) {
        if (NTL::IsOne(NTL::coeff(poly, i))) {
            v |= (uint64_t{1} << i);
        }
    }
    return v;
}

std::vector<uint8_t> serialize(const FE& x) {
    size_t n_bytes = field_element_bytes();
    std::vector<uint8_t> out(n_bytes, 0);
    uint64_t v = to_uint(x);
    for (size_t i = 0; i < n_bytes; i++) {
        out[i] = static_cast<uint8_t>((v >> (8 * i)) & 0xFF);
    }
    return out;
}

FE deserialize(const uint8_t* data, size_t len) {
    size_t expected = field_element_bytes();
    if (len != expected) {
        throw std::invalid_argument("field element byte length mismatch");
    }
    uint64_t v = 0;
    for (size_t i = 0; i < len; i++) {
        v |= uint64_t{data[i]} << (8 * i);
    }
    return from_uint(v);
}

} 
