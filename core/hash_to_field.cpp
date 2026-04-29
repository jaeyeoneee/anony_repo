// core/hash_to_field.cpp

#include "core/hash_to_field.h"

#include <sodium.h>
#include <cstring>
#include <stdexcept>

namespace epochess {

namespace {

struct SodiumInit {
    SodiumInit() {
        if (sodium_init() < 0) {
            throw std::runtime_error("libsodium init failed");
        }
    }
};
SodiumInit _sodium_init_instance;

}  

std::vector<uint8_t> sha256(const uint8_t* data, size_t len) {
    std::vector<uint8_t> out(crypto_hash_sha256_BYTES);
    crypto_hash_sha256(out.data(), data, len);
    return out;
}

std::vector<uint8_t> sha256(const std::vector<uint8_t>& data) {
    return sha256(data.data(), data.size());
}

NTL::GF2E hash_to_field(uint64_t identity_index, uint64_t nonce) {
    std::vector<uint8_t> buf;
    const char* tag = "EES|HF|";
    buf.insert(buf.end(), tag, tag + 7);
    for (int i = 0; i < 8; i++) {
        buf.push_back(static_cast<uint8_t>((identity_index >> (8 * i)) & 0xFF));
    }
    for (int i = 0; i < 8; i++) {
        buf.push_back(static_cast<uint8_t>((nonce >> (8 * i)) & 0xFF));
    }
    auto digest = sha256(buf);

    size_t n_bytes = field_element_bytes();
    uint64_t v = 0;
    for (size_t i = 0; i < n_bytes && i < 8; i++) {
        v |= uint64_t{digest[i]} << (8 * i);
    }
    unsigned s = field_degree();
    if (s < 64) {
        v &= (uint64_t{1} << s) - 1;
    }
    return from_uint(v);
}

NTL::GF2E hash_to_field_ds(const std::string& domain,
                           const std::vector<uint8_t>& data) {
    std::vector<uint8_t> buf;
    buf.insert(buf.end(), domain.begin(), domain.end());
    buf.push_back(0);
    buf.insert(buf.end(), data.begin(), data.end());
    auto digest = sha256(buf);
    size_t n_bytes = field_element_bytes();
    uint64_t v = 0;
    for (size_t i = 0; i < n_bytes && i < 8; i++) {
        v |= uint64_t{digest[i]} << (8 * i);
    }
    unsigned s = field_degree();
    if (s < 64) v &= (uint64_t{1} << s) - 1;
    return from_uint(v);
}

} 
