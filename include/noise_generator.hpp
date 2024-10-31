#pragma once

#include "common.hpp"

static inline float my_lerp(const float a, const float b, const float t)
{
    return (b - a) * t + a;
}

static inline float fade(const float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

static std::array<float, 2> rand_gradient(const float x, const float y)
{
    std::array<float, 2> gradient;

    float rand_degs = (float)(random() % 360);
    float rand_rads = rand_degs * (M_PI / 180);
    gradient[0] = cosf(rand_rads);
    gradient[1] = sinf(rand_rads);

    return gradient;
}

static float dot_grid_gradient(const float x0, const float y0, const float x1, const float y1)
{
    std::array<float, 2> gradient = rand_gradient(x0, y0);
    float dx = x1 - x0;
    float dy = y1 - y0;

    return (dx * gradient[0] + dy * gradient[1]);
}

static float perlin(const float x, const float y)
{
    float x0 = floorf(x);
    float x1 = x0 + 1.0f;
    float y0 = floorf(y);
    float y1 = y0 + 1.0f;

    float dx = x - x0;
    float dy = y - y0;

    float n0, n1, ix0, ix1;

    n0 = dot_grid_gradient(x0, y0, x, y);
    n1 = dot_grid_gradient(x1, y0, x, y);
    ix0 = my_lerp(n0, n1, dx);

    n0 = dot_grid_gradient(x0, y1, x, y);
    n1 = dot_grid_gradient(x1, y1, x, y);
    ix1 = my_lerp(n0, n1, dx);

    return my_lerp(ix0, ix1, dy);
}

static float octave_perlin(
    const float x,
    const float y,
    const uint8_t octaves,
    const float scale,
    const unsigned lo,
    const unsigned hi
)
{
    float noise = 0.0f;
    float amp = 1.0f;
    float max_amp = 0.0f;
    float freq = scale;

    for (uint8_t i = 0; i < octaves; ++i)
    {
        noise += perlin(x * freq, y * freq) * amp;
        max_amp += amp;
        amp *= 0.5f;
        freq *= 2.0f;
    }

    noise /= max_amp;
    noise = (noise * (hi - lo) / 2.0f) + ((hi + lo) / 2.0f);
    return noise;
}

static void output_noise_test()
{
    uint8_t alpha;
    const size_t width = 500;
    const size_t height = 500;
    const float scale = 0.05f;
    std::ofstream os("noise.ppm");

    os << "P6\n";
    os << width << " " << height << "\n";
    os << "255\n";

    for (size_t y = 0; y < height; ++y)
    {
        for (size_t x = 0; x < width; ++x)
        {
            alpha = (int)(perlin(x * scale, y * scale) * 255.0f);
            //alpha = octave_perlin(x, y, 4, scale, 0, 255);
            os << alpha << alpha << alpha;
        }
    }

    os.close();
}
