#pragma once

#include "common.hpp"
#include "camera.hpp"
#include "game_state.hpp"

enum class PlayerEvent
{
    DEL_BLOCK,
    MK_BLOCK
};

// Forward function declarations

void make_block_event(Camera &camera);
