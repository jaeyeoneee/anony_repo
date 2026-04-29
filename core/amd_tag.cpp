// core/amd_tag.cpp

#include "core/amd_tag.h"

#include <sodium.h>
#include <stdexcept>
#include <cstring>

namespace epochess {

std::vector<uint8_t> amd_keygen() {
    std::vector<uint8_t> key(AMD_KEY_BYTES);
    randombytes_buf(key.data(), key.size());
    return key;
}

std::vector<uint8_t>
amd_tag(const std::vector<uint8_t>& key,
        const std::vector<uint8_t>& message,
        size_t tag_bytes) {
    if (key.size() != AMD_KEY_BYTES) {
        throw std::invalid_argument("amd_tag: bad key size");
    }
    if (tag_bytes == 0 || tag_bytes > crypto_generichash_BYTES_MAX) {
        throw std::invalid_argument("amd_tag: bad tag size");
    }
    std::vector<uint8_t> tag(tag_bytes);
    if (crypto_generichash(tag.data(), tag.size(),
                           message.data(), message.size(),
                           key.data(), key.size()) != 0) {
        throw std::runtime_error("amd_tag: BLAKE2b failed");
    }
    return tag;
}

bool amd_verify(const std::vector<uint8_t>& key,
                const std::vector<uint8_t>& message,
                const std::vector<uint8_t>& tag) {
    std::vector<uint8_t> expected = amd_tag(key, message, tag.size());
    return sodium_memcmp(expected.data(), tag.data(), tag.size()) == 0;
}

} 
