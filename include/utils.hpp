#pragma once

#include "common.hpp"
#include "settings.hpp"

#define SET_BIT(mask, bit)    ((mask) |= (bit))
#define UNSET_BIT(mask, bit)  ((mask) &= (~bit))
#define IS_BIT_SET(mask, bit) (((mask) & (bit)) == (bit))
#define TOGGLE_BIT(mask, bit) ((mask) ^= (bit))

static uint32_t fnv1a_hash(const Vec3_t chunk_location, const Vec3_t block_location)
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

    Vec3_t v = qm_v3_add(chunk_location, block_location);
    hash_int((int)v.x);
    hash_int((int)v.y);
    hash_int((int)v.z);

    return hash;
}
