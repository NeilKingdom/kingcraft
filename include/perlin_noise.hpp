#pragma once

#include "common.hpp"

class PerlinNoise
{
public:
    // Special member functions
    PerlinNoise(const PerlinNoise &pn) = delete;
    PerlinNoise &operator=(const PerlinNoise &pn) = delete;
    PerlinNoise(PerlinNoise &&pn) = delete;
    PerlinNoise &operator=(PerlinNoise &&pn) = delete;

    // General
    static PerlinNoise &get_instance();

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
    // Member variables
    std::vector<int> permutations_table;

    // Special member functions
    PerlinNoise();
    ~PerlinNoise() = default;

    // General member functions
    static float gradient(const int hash, const float x, const float y, const float z);
    static float lerp(const float t, const float a, const float b);
    static float fade(const float t);
};
