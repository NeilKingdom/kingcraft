#pragma once

#include "common.hpp"

class PerlinNoise
{
public:
    // Special member functions
    PerlinNoise();
    ~PerlinNoise() = default;

    // General
    float perlin(const float x, const float y, const float z) const;
    float octave_perlin(
        const float x,
        const float y,
        const float z,
        const float scale,
        const uint8_t octaves,
        const unsigned lo,
        const unsigned hi
    ) const;

private:
    std::vector<int> m_permutations_table;

    // General member functions
    static float gradient(const int hash, const float x, const float y, const float z);
    static inline float lerp(const float t, const float a, const float b);
    static inline float fade(const float t);
};
