#pragma once

#include "common.hpp"
#include "constants.hpp"

class Player
{
public:
    float speed = KC::PLAYER_BASE_SPEED;

    // Special member functions
    Player() = default;
    ~Player() = default;
};
