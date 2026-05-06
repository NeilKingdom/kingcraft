#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "settings.hpp"
#include "chunk_manager.hpp"

class Camera
{
public:
    // Member variables
    Vec3_t v_eye;                   // Camera location
    Vec3_t v_look_dir;              // Unit vector representing direction camera is facing on current frame
    std::shared_ptr<Mat4_t> m_view; // View matrix (4x4)

    // Special member functions
    Camera();
    ~Camera() = default;
    Camera(const Camera &camera) = default;
    Camera &operator=(const Camera &camera) = default;
    Camera(Camera &&camera) = default;
    Camera &operator=(Camera &&camera) = default;

    // General
    void calculate_view_matrix();
    void update_rotation_from_pointer(const KCWindow &win);
    bool is_chunk_in_visible_radius(const Vec3_t chunk_location) const;
    std::optional<Block> cast_ray(const uint8_t n_iters = 5) const;

private:
    // Member variables
    float camera_yaw;   // Rotation about the z axis
    float camera_pitch; // Rotation about the y axis
};
