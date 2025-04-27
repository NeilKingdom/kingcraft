#include "player.hpp"

Player::Player() :
    curr_speed(KC::PLAYER_SPEED_FACTOR)
{
    curr_move_mode = MovementMode::WALKING;
}
