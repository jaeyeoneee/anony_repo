// core/field.h — Finite field F_{2^s} wrapper over NTL::GF2E.


#pragma once

#include <NTL/GF2E.h>
#include <NTL/GF2EX.h>
#include <NTL/GF2X.h>
#include <NTL/GF2XFactoring.h>

#include <cstdint>
#include <vector>
#include <string>

namespace epochess {

// Initialize F_{2^s}.
void init_field(unsigned s);

// Current extension degree.
unsigned field_degree();

// Size of the field: q = 2^s.
uint64_t field_size();

// Byte size of a field element.
size_t field_element_bytes();

// -------- Element operations (convenience wrappers) --------
using FE = NTL::GF2E;

// Random nonzero element of F_{2^s}.
FE random_nonzero_element();

// Random element.
FE random_element();

// Convert an integer 0 <= v < 2^s to a field element
FE from_uint(uint64_t v);

// Convert a field element to its integer representation (<= 2^s - 1).
uint64_t to_uint(const FE& x);

// Serialize a field element to `s/8` bytes (little-endian packed).
std::vector<uint8_t> serialize(const FE& x);

// Deserialize bytes into a field element. Throws on length mismatch.
FE deserialize(const uint8_t* data, size_t len);

inline bool is_nonzero(const FE& x) { return !NTL::IsZero(x); }

} 
