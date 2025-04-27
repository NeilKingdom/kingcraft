#pragma once

#include "common.hpp"
#include "constants.hpp"

enum class MovementMode
{
    WALKING,
    RUNNING,
    FLYING,
    SWIMMING
};

class Player
{
public:
    float curr_speed;
    MovementMode curr_move_mode;

    // Special member functions
    Player();
    ~Player() = default;
};
