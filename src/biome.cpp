#include "biome.hpp"

Biome plains_biome = {
    .scale = 0.01f,
    .octaves = 3,
    .lo = KC::SEA_LEVEL,
    .hi = KC::SEA_LEVEL + (KC::CHUNK_SIZE * 3)
};

Biome mount_biome = {
    .scale = 0.04f,
    .octaves = 4,
    .lo = KC::SEA_LEVEL,
    .hi = KC::SEA_LEVEL + (KC::CHUNK_SIZE * 6)
};

Biome ocean_biome = {
    .scale = 0.02f,
    .octaves = 2,
    .lo = 0,
    .hi = KC::SEA_LEVEL
};

float sample_biome_height(const vec2 point)
{
    PerlinNoise &pn = PerlinNoise::get_instance();

    // TODO: point grabs biome from biome map, then that dictates which biome to select for sampling
    Biome biome = plains_biome;
    return pn.octave_perlin(
        point[0], point[1], 0.0f,
        biome.scale,
        biome.octaves,
        biome.lo, biome.hi);
}
