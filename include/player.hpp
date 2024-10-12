#pragma once

#include "common.hpp"

class Player
{
public:
    static constexpr float PLAYER_BASE_SPEED = 0.2f; // Normalized speed

    float speed;

    Player();
    ~Player() = default;

private:
};
