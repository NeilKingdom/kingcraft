#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "game_state.hpp"
#include "chunk_column.hpp"
#include "block.hpp"

struct CullingFrustum
{
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
    vec3 v_eye;                                    // Camera's origin
    vec3 v_look_dir;                               // The direction that the camera is facing on the current frame
    std::shared_ptr<std::array<float, 16>> m_view; // View matrix

    // Special member functions
    Camera();
    ~Camera() = default;

    // General
    void update_rotation_from_pointer(const KCWindow &win);
    std::optional<Block> cast_ray(const std::set<ChunkColumn> &chunks, const unsigned n_iters) const;
    void calculate_view_matrix();
    CullingFrustum get_frustum_coords(size_t render_distance);

private:
    float camera_yaw;   // Rotation about the z axis
    float camera_pitch; // Rotation about the y axis
};
