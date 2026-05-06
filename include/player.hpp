#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "mesh.hpp"
#include "camera.hpp"

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
    // Member variables
    Vec3_t v_vel;
    MovementMode curr_move_mode;

    // Special member functions
    Player(const Player &player) = delete;
    Player &operator=(const Player &player) = delete;
    Player(Player &&player) = delete;
    Player &operator=(Player &&player) = delete;

    // General
    static Player &get_instance();

private:
    // Special member functions
    Player();
    ~Player() = default;
};
