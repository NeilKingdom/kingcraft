#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "settings.hpp"
#include "chunk.hpp"

class Camera
{
public:
    vec3 v_eye; // Camera's origin
    vec3 v_look_dir; // Unit vector representing direction camera is facing on current frame
    std::shared_ptr<std::array<float, 16>> m_view; // View matrix (4x4)

    // Special member functions
    Camera();
    ~Camera() = default;

    // General
    void calculate_view_matrix();
    void update_rotation_from_pointer(const KCWindow &win);
    bool is_chunk_in_visible_radius(const vec2 chunk_location) const;
    //std::optional<Block> cast_ray(const std::vector<std::shared_ptr<Chunk>> &chunks, const unsigned n_iters) const;

private:
    float camera_yaw;   // Rotation about the z axis
    float camera_pitch; // Rotation about the y axis
};
