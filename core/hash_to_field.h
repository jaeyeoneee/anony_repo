// core/hash_to_field.h — Random-oracle hash H: N -> F_{2^s}

#pragma once

#include "core/field.h"

#include <cstdint>
#include <string>
#include <vector>

namespace epochess {

// Hash an identity index i into a field element y_i = H(i) in F_{2^s}.
NTL::GF2E hash_to_field(uint64_t identity_index, uint64_t nonce = 0);

// Domain-separated variant for system-level hashes (e.g., committee IDs).
NTL::GF2E hash_to_field_ds(const std::string& domain, const std::vector<uint8_t>& data);

// Raw SHA-256 (32 bytes out). Useful for integrity tags, commit-reveal, etc.
std::vector<uint8_t> sha256(const uint8_t* data, size_t len);
std::vector<uint8_t> sha256(const std::vector<uint8_t>& data);

} 
