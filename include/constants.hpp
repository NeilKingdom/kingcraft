#pragma once

#include <chrono>
#include <stdint.h>
#include "transforms.h"

namespace KC
{
    using namespace std::chrono;

    // KC coordinate system directional unit vectors
    static constexpr vec3 v_fwd   = { 1.0f, 0.0f, 0.0f };
    static constexpr vec3 v_right = { 0.0f, 1.0f, 0.0f };
    static constexpr vec3 v_up    = { 0.0f, 0.0f, 1.0f };

    // General constants
    static constexpr unsigned CUBE_FACES = 6;
    static constexpr unsigned TEX_ATLAS_NCOLS = 16;
    static constexpr auto SEC_AS_MS = duration_cast<milliseconds>(seconds(1));

    // Normalized scaling factors
    static constexpr float CAMERA_SPEED_FACTOR = 0.5f;
    static constexpr float PLAYER_SPEED_FACTOR = 0.5f;

    // Measurements (1 block => 1 unit of measurement)
    static constexpr unsigned PLAYER_HEIGHT = 2;
    static constexpr unsigned MAX_BLOCK_HEIGHT = UINT8_MAX;
    static constexpr unsigned SEA_LEVEL = (unsigned)(MAX_BLOCK_HEIGHT / 2);
};
