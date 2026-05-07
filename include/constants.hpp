#pragma once

#include <chrono>
#include <stdint.h>
#include "/home/neil/devel/projects/quikmafs/lib/quikmafs.h"

namespace KC
{
    using namespace std::chrono;

    // KC coordinate system directional unit vectors
    static constexpr Vec3_t v_fwd   = { .v = { 1.0f, 0.0f, 0.0f }};
    static constexpr Vec3_t v_right = { .v = { 0.0f, 1.0f, 0.0f }};
    static constexpr Vec3_t v_up    = { .v = { 0.0f, 0.0f, 1.0f }};

    // General constants
    static constexpr unsigned CHUNK_SIZE = 16;
    static constexpr unsigned CUBE_FACES = 6;
    static constexpr unsigned TEX_ATLAS_NCOLS = 16;
    static constexpr auto SEC_AS_MS = duration_cast<milliseconds>(seconds(1));

    // Normalized scaling factors
    static constexpr float CAMERA_SPEED_FACTOR = 0.5f;
    static constexpr float PLAYER_SPEED_FACTOR = 7.0f;

    // Measurements (1 block => 1 unit of measurement)
    static constexpr unsigned PLAYER_HEIGHT = 2;
    static constexpr unsigned MAX_BLOCK_HEIGHT = UINT8_MAX;
    static constexpr unsigned SEA_LEVEL = (unsigned)((MAX_BLOCK_HEIGHT + 1) / 2);
};
