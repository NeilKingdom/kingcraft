#pragma once

#include <atomic>

#include "common.hpp"
#include "player.hpp"

namespace GameState
{
    static std::atomic<bool> is_running = true;
    static std::atomic<float> fov = 90.0f;
    static std::atomic<float> aspect = 9.0f / 16.0f;

    static Player player = Player();
};
