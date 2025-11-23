#pragma once

#include "common.hpp"
#include "settings.hpp"

#define SET_BIT(mask, bit)    ((mask) |= (bit))
#define UNSET_BIT(mask, bit)  ((mask) &= (~bit))
#define IS_BIT_SET(mask, bit) (((mask) & (bit)) == (bit))
#define TOGGLE_BIT(mask, bit) ((mask) ^= (bit))

// Kind of bad programming, but intended to be used to compare std::array<float, 3> with vec3 from liblac
#define V3_EQ(a, b) \
    (a[0]) == (b[0]) && \
    (a[1]) == (b[1]) && \
    (a[2]) == (b[2])

static uint32_t fnv1a_hash(const vec3 chunk_location, const vec3 block_location)
{
    const uint32_t fnv_offset = 2166136261;
    const uint32_t fnv_prime = 16777619;
    uint32_t hash = fnv_offset;

    auto hash_int = [&](int value) {
        // Split int into 4 individual bytes and hash each
        for (int i = 0; i < 4; ++i) {
            uint8_t b = (value >> (i * 8)) & 0xFF;
            hash ^= b;
            hash *= fnv_prime;
        }
    };

    vec3 v{};
    lac_add_vec3(v, chunk_location, block_location);

    hash_int((int)v[0]);
    hash_int((int)v[1]);
    hash_int((int)v[2]);

    return hash;
}

