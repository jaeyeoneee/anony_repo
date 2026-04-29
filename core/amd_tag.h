// core/amd_tag.h — Algebraic Manipulation Detection (AMD) tags.

#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

namespace epochess {

constexpr size_t AMD_KEY_BYTES = 32;
constexpr size_t AMD_TAG_BYTES_DEFAULT = 8;   // 64-bit detection
constexpr size_t AMD_TAG_BYTES_HI = 16;       // 128-bit detection

// Generate a random AMD key.
std::vector<uint8_t> amd_keygen();

// Produce a tag of `tag_bytes` bytes.
std::vector<uint8_t>
amd_tag(const std::vector<uint8_t>& key,
        const std::vector<uint8_t>& message,
        size_t tag_bytes = AMD_TAG_BYTES_DEFAULT);

// Verify a tag.
bool amd_verify(const std::vector<uint8_t>& key,
                const std::vector<uint8_t>& message,
                const std::vector<uint8_t>& tag);

}  
