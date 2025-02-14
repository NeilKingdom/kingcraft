#include "perlin_noise.hpp"

/**
 * @brief Default constructor for PerlinNoise generator.
 * @since 02-01-2025
 * @param[in] octaves Optional parameter which specifies the amount of octaves used in octave_perlin()
 */
PerlinNoise::PerlinNoise()
{
	m_permutations_table.resize(256);
    for (size_t i = 0; i < m_permutations_table.size(); ++i)
    {
        m_permutations_table[i] = random() % 256;
    }
	m_permutations_table.insert(
        m_permutations_table.end(),
        m_permutations_table.begin(),
        m_permutations_table.end()
    );
}

/**
 * @brief Samples height at the specified location and returns the normalized value between -1 and 1.
 * @since 02-01-2025
 * @param x[in] The x component of the sampled coordinate
 * @param y[in] The y component of the sampled coordinate
 * @param z[in] The z component of the sampled coordinate
 * @returns A normalized value between -1 and 1 representing the sampled point
 */
float PerlinNoise::perlin(const float x, const float y, const float z)
{
    float _x = x;
    float _y = y;
    float _z = z;

	// Find the unit cube that contains the point
	int X = (int)std::floorf(_x) & 255;
	int Y = (int)std::floorf(_y) & 255;
	int Z = (int)std::floorf(_z) & 255;

	// Find relative x, y, z of point in cube
	_x -= std::floorf(_x);
	_y -= std::floorf(_y);
	_z -= std::floorf(_z);

	// Compute fade curves for each of x, y, z
	float u = fade(_x);
	float v = fade(_y);
	float w = fade(_z);

	// Hash coordinates of the 8 cube corners
	int A  = m_permutations_table[X]     + Y;
	int AA = m_permutations_table[A]     + Z;
	int AB = m_permutations_table[A + 1] + Z;
	int B  = m_permutations_table[X + 1] + Y;
	int BA = m_permutations_table[B]     + Z;
	int BB = m_permutations_table[B + 1] + Z;

	// Add blended results from 8 corners of cube
	float noise = lerp(
        w, lerp(
            v, lerp(
                u, gradient(m_permutations_table[AA], _x, _y, _z),
                gradient(m_permutations_table[BA], _x - 1, _y, _z)
            ),
            lerp(
                u, gradient(m_permutations_table[AB], _x, _y - 1, _z),
                gradient(m_permutations_table[BB], _x - 1, _y - 1, _z)
            )
        ),
        lerp(
            v, lerp(
                u, gradient(m_permutations_table[AA + 1], _x, _y, _z - 1),
                gradient(m_permutations_table[BA + 1], _x - 1, _y, _z - 1)
            ),
            lerp(
                u, gradient(m_permutations_table[AB + 1], _x, _y - 1, _z - 1),
                gradient(m_permutations_table[BB + 1], _x - 1, _y - 1, _z - 1)
            )
        )
    );

    return noise;
}

/**
 * @brief Combines multiple layered instances of perlin noise at various frequencies and amplitudes.
 * @since 02-01-2025
 * @param[in] x The x component of the sampled coordinate
 * @param[in] y The y component of the sampled coordinate
 * @param[in] z The z component of the sampled coordinate
 * @param[in] scale Scaling factor for noise
 * @param[in] octaves The amount of samples to take
 * @param[in] lo The minimum value that can be returned
 * @param[in] hi The maximum value that can be returned
 * @returns A value between __lo__ and __hi__ for the sampled point
 */
float PerlinNoise::octave_perlin(
    const float x,
    const float y,
    const float z,
    const float scale,
    const uint8_t octaves,
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

// TODO: Document.
/**
 * @brief Calculate the gradient vector for a given grid point.
 * @since 02-01-2025
 * @param[in] hash
 * @param[in] x [TODO:parameter]
 * @param[in] y [TODO:parameter]
 * @param[in] z [TODO:parameter]
 * @return [TODO:return]
 */
float PerlinNoise::gradient(const int hash, const float x, const float y, const float z)
{
	int h = hash & 0x0F;
	// Convert lower 4 bits of hash into 12 gradient directions
	float u = (h < 8) ? x : y;
    float v = (h < 4) ? y : h == 12 || h == 14 ? x : z;
	return (((h & 1) == 0) ? u : -u) + (((h & 2) == 0) ? v : -v);
}

/**
 * @brief Linearly interpolates between two points __a__ and __b__ at the normalized distance __t__.
 * @since 02-01-2025
 * @param[in] t The normalized distance to be interpolated between __a__ and __b__
 * @param[in] a The starting value used for the interpolation
 * @param[in] b The ending value used for the interpolation
 * @returns The value that corresponds to the interpolated distance __t__ which lies between values __a__ and __b__
 */
inline float PerlinNoise::lerp(const float t, const float a, const float b)
{
    return (b - a) * t + a;
}

/**
 * @brief Provides a smooth step transition given __t__, which represents the normalized distance between two points.
 * @since 02-01-2025
 * @param[in] t A normalized value between 0 and 1 representing the distance between two points
 * @returns A value between 0 and 1 which represents the smoothed value of t
 */
inline float PerlinNoise::fade(const float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}
