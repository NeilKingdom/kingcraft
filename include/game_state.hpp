#pragma once

#include <atomic>

#include "common.hpp"
#include "player.hpp"

namespace GameState
{
    static std::atomic<bool> is_running = true;

    static Player player = Player();
};
