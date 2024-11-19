#pragma once

#include "common.hpp"
#include "constants.hpp"

class Player
{
public:
    float speed;
    float height;

    // Special member functions
    Player();
    ~Player() = default;
};
