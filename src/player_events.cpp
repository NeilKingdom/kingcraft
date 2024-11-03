#include "player_events.hpp"

void make_block_event(Camera &camera)
{
    const unsigned ray_steps = 3;
    GameState &game = GameState::get_instance();

    vec3 u_look_dir = {};
    lac_normalize_vec3(&u_look_dir, camera.v_look_dir);

    for (unsigned i = 0; i < ray_steps; ++i)
    {

    }
}
