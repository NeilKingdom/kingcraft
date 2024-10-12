#pragma once

#include "common.hpp"
#include "game_state.hpp"

class Camera
{
public:
    static constexpr float CAMERA_ROTATION_SPEED = 0.5f; // 0.0 - 1.0

    const vec3 v_fwd   = { 1.0f, 0.0f, 0.0f };
    const vec3 v_right = { 0.0f, 1.0f, 0.0f };
    const vec3 v_up    = { 0.0f, 0.0f, 1.0f };

    const float fov;                                // Field of view
    const float aspect;                             // Aspect ratio
    const float znear;                              // Near clipping plane
    const float zfar;                               // Far clipping plane

    vec3 v_eye;                                     // Camera's origin point
    vec3 v_fwd_vel;                                 // Forward velocity vector
    vec3 v_right_vel;                               // Right velocity vector
    std::shared_ptr<std::array<float, 16>> m_view;  // View matrix

    // Special member functions
    Camera(const float fov = 90.0f, const float aspect = (16.0f / 9.0f), const float znear = 1.0f, const float zfar = 1000.0f);
    ~Camera() = default;

    // General
    void update_velocity();
    void update_rotation_from_pointer(const XObjects &x_objs);
    void calculate_view_matrix();

private:
    float camera_yaw;       // Rotation about the z axis
    float camera_pitch;     // Rotation about the y axis

    vec3 v_look_dir;        // The direction that the camera is facing on the current frame
    vec3 v_new_look_dir;    // The direction that the camera will be facing on the next frame

    mat4 m_point_at;        // The point-at matrix which becomes the view matrix after inversion
    mat4 m_cam_rot;         // Rotation matrix used for transforming v_look_dir
};
