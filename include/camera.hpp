#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "game_state.hpp"

struct CullingFrustum
{
    std::array<float, 2> v_eye;
    std::array<float, 2> v_left;
    std::array<float, 2> v_right;
};

class Camera
{
public:
    const vec3 v_fwd   = { 1.0f, 0.0f, 0.0f };
    const vec3 v_right = { 0.0f, 1.0f, 0.0f };
    const vec3 v_up    = { 0.0f, 0.0f, 1.0f };

    vec3 v_eye;                                    // Camera's origin
    vec3 v_look_dir;                               // The direction that the camera is facing on the current frame
    std::shared_ptr<std::array<float, 16>> m_view; // View matrix

    // Special member functions
    Camera();
    ~Camera() = default;

    // General
    void update_rotation_from_pointer(const KCWindow &win);
    void calculate_view_matrix();
    CullingFrustum get_frustum_coords(uint8_t distance);

private:
    float camera_yaw;   // Rotation about the z axis
    float camera_pitch; // Rotation about the y axis
};
