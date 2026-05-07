#include "player.hpp"

Player::Player() :
    v_vel{},
    v_jump{},
    curr_move_mode(MovementMode::WALKING)
{}

Player &Player::get_instance()
{
    static Player player;
    return player;
}

void Player::update_plyr_movement(const Camera &camera)
{
    Vec3_t v_fwd = camera.v_look_dir;
    Vec3_t v_right = qm_v3_norm(qm_v3_cross(KC::v_up, v_fwd));
    this->v_vel = {};

    if (IS_BIT_SET(key_mask, KeyAction::PLYR_FWD))
    {
        this->v_vel = qm_v3_add(this->v_vel, v_fwd);
    }
    if (IS_BIT_SET(key_mask, KeyAction::PLYR_BACK))
    {
        this->v_vel = qm_v3_sub(this->v_vel, v_fwd);
    }
    if (IS_BIT_SET(key_mask, KeyAction::PLYR_LEFT))
    {
        this->v_vel = qm_v3_add(this->v_vel, v_right);
    }
    if (IS_BIT_SET(key_mask, KeyAction::PLYR_RIGHT))
    {
        this->v_vel = qm_v3_sub(this->v_vel, v_right);
    }
    if (IS_BIT_SET(key_mask, KeyAction::PLYR_UP))
    {
        if (this->curr_move_mode == MovementMode::WALKING)
        {
            // Reset jump vector
            if (this->state == JumpState::RESET)
            {
                this->state = JumpState::JUMP_KEY_PRESS;
                reset_v_jump();
            }
        }
        else if (this->curr_move_mode == MovementMode::FLYING)
        {
            this->v_vel = qm_v3_add(this->v_vel, KC::v_up);
        }
    }
    if (IS_BIT_SET(key_mask, KeyAction::PLYR_DOWN))
    {
        if (this->curr_move_mode == MovementMode::FLYING)
        {
            this->v_vel = qm_v3_sub(this->v_vel, KC::v_up);
        }
    }

    this->v_vel = qm_v3_norm(this->v_vel);
    float magnitude = qm_v3_len(this->v_vel);
    if (magnitude > 0.0f)
    {
        this->v_vel = qm_v3_scale(this->v_vel, KC::PLAYER_SPEED_FACTOR);
    }
}

void Player::reset_v_jump()
{
    const float jump_velocity = 10.0f;
    this->v_jump = qm_v3_scale(KC::v_up, jump_velocity);
}
