#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "settings.hpp"
#include "chunk.hpp"

class Frustum2D
{
public:
    std::array<float, 2> v_eye;
    std::array<float, 2> v_left;
    std::array<float, 2> v_right;

    /**
     * @brief Checks if a point rests within the bounds of the camera's viewing frustum.
     * @since 01-03-2025
     * @param[in] point The point being checked
     * @returns True if the point lies within the viewing frustum, otherwise returns false
     */
    bool is_point_within(const vec2 point)
    {
        float areaPAB = (v_eye[0]   - point[0]) * (v_left[1]  - point[1]) - (v_eye[1]   - point[1]) * (v_left[0]  - point[0]);
        float areaPBC = (v_left[0]  - point[0]) * (v_right[1] - point[1]) - (v_left[1]  - point[1]) * (v_right[0] - point[0]);
        float areaPCA = (v_right[0] - point[0]) * (v_eye[1]   - point[1]) - (v_right[1] - point[1]) * (v_eye[0]   - point[0]);

        bool has_neg = (areaPAB < 0) || (areaPBC < 0) || (areaPCA < 0);
        bool has_pos = (areaPAB > 0) || (areaPBC > 0) || (areaPCA > 0);

        return !(has_neg && has_pos);
    }
};

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
    void update_rotation_from_pointer(const KCWindow &win, const vec2 pointer_pos);
    Frustum2D get_frustum2D(const size_t render_distance) const;
    //std::optional<Block> cast_ray(const std::vector<std::shared_ptr<Chunk>> &chunks, const unsigned n_iters) const;

private:
    float camera_yaw;   // Rotation about the z axis
    float camera_pitch; // Rotation about the y axis
};
