#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "settings.hpp"

#define SET_BIT(mask, bit)    ((mask) |= (bit))
#define UNSET_BIT(mask, bit)  ((mask) &= (~bit))
#define IS_BIT_SET(mask, bit) (((mask) & (bit)) == (bit))
#define TOGGLE_BIT(mask, bit) ((mask) ^= (bit))

static inline uint32_t hash32(const uint32_t x)
{
    uint32_t _x = x;

    _x ^= _x >> 16;
    _x *= 0x7FEB352D;
    _x ^= _x >> 15;
    _x *= 0x846CA68B;
    _x ^= _x >> 16;

    return _x;
}

static inline uint32_t world_hash(const Vec3_t chunk_location, const Vec3_t block_location)
{
    Settings &settings = Settings::get_instance();

    const Vec3_t world_location = { .v = {
        (chunk_location.x * KC::CHUNK_SIZE) + block_location.x,
        (chunk_location.y * KC::CHUNK_SIZE) + block_location.y,
        (chunk_location.z * KC::CHUNK_SIZE) + block_location.z,
    }};

    uint32_t hash =
        (uint32_t)world_location.x * 73856093 ^
        (uint32_t)world_location.y * 19349663 ^
        (uint32_t)world_location.z * 83492791 ^
        settings.seed;

    return hash32(hash);
}
