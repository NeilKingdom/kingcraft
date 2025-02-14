#pragma once

#include "common.hpp"

class PerlinNoise
{
public:
    // Special member functions
    PerlinNoise();
    ~PerlinNoise() = default;

    // General
    float perlin(const float x, const float y, const float z);
    float octave_perlin(const float x, const float y, const float z, const float scale, const uint8_t octaves, const unsigned lo, const unsigned hi);

private:
    std::vector<int> m_permutations_table;

    // General member functions
    float gradient(const int hash, const float x, const float y, const float z);
    inline float lerp(const float t, const float a, const float b);
    inline float fade(const float t);
};
