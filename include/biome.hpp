#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "perlin_noise.hpp"

enum class BiomeType
{
    PLAINS,
    DESERT,
    MOUNTAIN,
    OCEAN,
};

struct Biome
{
    float scale;
    uint8_t octaves;
    unsigned lo;
    unsigned hi;
};

extern Biome plains_biome;
extern Biome mount_biome;
extern Biome ocean_biome;

float sample_biome_height(const vec2 point);
