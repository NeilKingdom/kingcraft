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

enum class JumpState
{
    RESET,
    JUMP_KEY_PRESS,
};

class Player
{
public:
    JumpState state = JumpState::RESET;

    // Member variables
    Vec3_t v_vel;
    Vec3_t v_jump;
    MovementMode curr_move_mode;

    // Special member functions
    Player(const Player &player) = delete;
    Player &operator=(const Player &player) = delete;
    Player(Player &&player) = delete;
    Player &operator=(Player &&player) = delete;

    // General
    static Player &get_instance();
    void update_plyr_movement(const Camera &camera);

private:
    // Special member functions
    Player();
    ~Player() = default;

    // General
    void reset_v_jump();
};
