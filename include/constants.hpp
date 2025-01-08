#pragma once

#include "transforms.h"

namespace KC
{
    static constexpr vec3 v_fwd   = { 1.0f, 0.0f, 0.0f };
    static constexpr vec3 v_right = { 0.0f, 1.0f, 0.0f };
    static constexpr vec3 v_up    = { 0.0f, 0.0f, 1.0f };

    static constexpr long     SEC_AS_NANO = 1000L * 1000L * 1000L;
    static constexpr unsigned CUBE_FACES = 6;
    static constexpr unsigned ATLAS_TEX_SIZE = 16;                    // Number of texture columns in the atlas

    static constexpr float    CAMERA_ROTATION_SPEED = 0.5f;           // Normalized rotation speed
    static constexpr float    PLAYER_BASE_SPEED = 0.5f;               // Normalized speed
    static constexpr float    PLAYER_HEIGHT = 2.0f;                   // Height (in blocks)
};
