#include "player.hpp"

Player::Player() :
    v_vel{},
    curr_move_mode(MovementMode::WALKING)
{}

Player &Player::get_instance()
{
    static Player player;
    return player;
}
