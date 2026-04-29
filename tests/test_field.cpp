// tests/test_field.cpp — F_{2^s} unit tests.
#include "core/field.h"

#include <cassert>
#include <cstdio>

using namespace epochess;

int main() {
    for (unsigned s : {8u, 12u, 16u, 24u, 32u}) {
        init_field(s);
        assert(field_degree() == s);
        assert(field_size() == (uint64_t{1} << s));
        // serialize/deserialize roundtrip
        for (int i = 0; i < 100; i++) {
            auto x = random_element();
            auto bytes = serialize(x);
            auto y = deserialize(bytes.data(), bytes.size());
            assert(x == y);
        }
        // random_nonzero never returns zero
        for (int i = 0; i < 1000; i++) {
            auto x = random_nonzero_element();
            assert(!NTL::IsZero(x));
        }
        std::printf("[field s=%u] PASSED\n", s);
    }
    return 0;
}
