#include "perlin_noise.hpp"

PerlinNoise::PerlinNoise()
{
	permutations_table.resize(256);
    for (size_t i = 0; i < permutations_table.size(); ++i)
    {
        permutations_table[i] = random() % 256;
    }
	permutations_table.insert(permutations_table.end(), permutations_table.begin(), permutations_table.end());
}

float PerlinNoise::perlin(const float x, const float y, const float z)
{
    float _x = x;
    float _y = y;
    float _z = z;

	// Find the unit cube that contains the point
	int X = (int)std::floorf(_x) & 255;
	int Y = (int)std::floorf(_y) & 255;
	int Z = (int)std::floorf(_z) & 255;

	// Find relative x, y,z of point in cube
	_x -= std::floorf(_x);
	_y -= std::floorf(_y);
	_z -= std::floorf(_z);

	// Compute fade curves for each of x, y, z
	float u = fade(_x);
	float v = fade(_y);
	float w = fade(_z);

	// Hash coordinates of the 8 cube corners
	int A  = permutations_table[X]     + Y;
	int AA = permutations_table[A]     + Z;
	int AB = permutations_table[A + 1] + Z;
	int B  = permutations_table[X + 1] + Y;
	int BA = permutations_table[B]     + Z;
	int BB = permutations_table[B + 1] + Z;

	// Add blended results from 8 corners of cube
	float noise = lerp(
        w, lerp(
            v, lerp(
                u, gradient(permutations_table[AA], _x, _y, _z),
                gradient(permutations_table[BA], _x - 1, _y, _z)
            ),
            lerp(
                u, gradient(permutations_table[AB], _x, _y - 1, _z),
                gradient(permutations_table[BB], _x - 1, _y - 1, _z)
            )
        ),
        lerp(
            v, lerp(
                u, gradient(permutations_table[AA + 1], _x, _y, _z - 1),
                gradient(permutations_table[BA + 1], _x - 1, _y, _z - 1)
            ),
            lerp(
                u, gradient(permutations_table[AB + 1], _x, _y - 1, _z - 1),
                gradient(permutations_table[BB + 1], _x - 1, _y - 1, _z - 1)
            )
        )
    );

    return noise;
}

float PerlinNoise::octave_perlin(
    const float x,
    const float y,
    const float z,
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
        noise += perlin(x * freq, y * freq, z * freq) * amp;
        max_amp += amp;
        amp *= 0.5f;
        freq *= 2.0f;
    }

    noise /= max_amp;
    noise = (noise * (hi - lo) / 2.0f) + ((hi + lo) / 2.0f);
    return noise;
}

inline float PerlinNoise::gradient(const int hash, const float x, const float y, const float z)
{
	int h = hash & 0x0F;
	// Convert lower 4 bits of hash into 12 gradient directions
	float u = (h < 8) ? x : y;
    float v = (h < 4) ? y : h == 12 || h == 14 ? x : z;
	return (((h & 1) == 0) ? u : -u) + (((h & 2) == 0) ? v : -v);
}

inline float PerlinNoise::lerp(const float t, const float a, const float b)
{
    return (b - a) * t + a;
}

float PerlinNoise::fade(const float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}
