#pragma once

#include "common.hpp"

class PerlinNoise
{
public:
    // Special member functions
    PerlinNoise(const uint8_t octaves = 3);
    ~PerlinNoise() = default;

    // General
    float perlin(const float x, const float y, const float z);
    float octave_perlin(const float x, const float y, const float z, const float scale, const unsigned lo, const unsigned hi);

private:
    // General member functions
    static float gradient(const int hash, const float x, const float y, const float z);
    static inline float lerp(const float t, const float a, const float b);
    static inline float fade(const float t);

    uint8_t m_octaves;
    std::vector<int> m_permutations_table;
};
