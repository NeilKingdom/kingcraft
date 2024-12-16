/**
 * @file camera.cpp
 * @author Neil Kingdom
 * @version 1.0
 * @since 02-03-2024
 * @brief Camera class used to create and update the view matrix.
 */

#include "camera.hpp"

/**
 * @brief Default constructor for the Camera class.
 * @since 02-03-2024
 */
Camera::Camera() :
    camera_yaw(0.0f),
    camera_pitch(0.0f),
    v_eye{},
    m_view(std::make_shared<std::array<float, 16>>())
{
    std::memcpy(v_look_dir, v_fwd, sizeof(v_look_dir));
}

/**
 * @brief Update the Camera's rotation matrix given the position of the mouse/pointer on screen.
 * @since 02-03-2024
 * @param[in] win A reference to the application's window
 */
void Camera::update_rotation_from_pointer(const KCWindow &win)
{
    int win_loc_x, win_loc_y, win_edge_x, win_edge_y;
    int win_off_x, win_off_y, root_off_x, root_off_y;
    float center_x, center_y, norm_dx, norm_dy, dist_x, dist_y;
    unsigned inop;
    Window wnop;

    XQueryPointer(
        win.dpy, win.win, &wnop, &wnop,
        &root_off_x, &root_off_y, &win_off_x, &win_off_y,
        &inop
    );

    win_loc_x  = win.xwa.x;
    win_loc_y  = win.xwa.y;
    win_edge_x = win.xwa.x + win.xwa.width;
    win_edge_y = win.xwa.y + win.xwa.height;

    // Cursor is outside window. Warp to nearest edge.
    if (root_off_x < win_loc_x || root_off_x > win_edge_x
        || root_off_y < win_loc_y || root_off_y > win_edge_y)
    {
        dist_x = std::min(std::abs(root_off_x - win_loc_x), std::abs(root_off_x - win_edge_x));
        dist_y = std::min(std::abs(root_off_y - win_loc_y), std::abs(root_off_y - win_edge_y));
        win_off_x = (dist_x == std::abs(root_off_x - win_loc_y)) ? 0 : win.xwa.width - 1;
        win_off_y = (dist_y == std::abs(root_off_y - win_loc_x)) ? 0 : win.xwa.height - 1;
        XWarpPointer(win.dpy, None, win.win, 0, 0, 0, 0, win_off_x, win_off_y);
    }

    center_x = (float)win.xwa.width / 2.0f;
    center_y = (float)win.xwa.height / 2.0f;
    norm_dx  = (center_x - (float)win_off_x) / (float)win.xwa.width;
    norm_dy  = (center_y - (float)win_off_y) / (float)win.xwa.height;

    // Convert from pixel space to degrees
    camera_yaw += norm_dx * 180.0f * KC::CAMERA_ROTATION_SPEED;
    camera_pitch += norm_dy * 180.0f * KC::CAMERA_ROTATION_SPEED;
    camera_pitch = std::clamp(camera_pitch, -89.0f, 89.0f);

    // Warp back to center of screen
    XWarpPointer(
        win.dpy, win.win, win.win, 0, 0,
        win.xwa.width, win.xwa.height, center_x, center_y
    );
}

/**
 * @brief Calculates the view matrix based on the look direction of the camera.
 * @since 02-03-2024
 */
void Camera::calculate_view_matrix()
{
    vec3 v3_new_look_dir = {};
    vec4 v4_new_look_dir = { v_fwd[0], v_fwd[1], v_fwd[2], 1.0f };
    mat4 m_point_at = {};

    mat4 m_yaw = {};
    mat4 m_pitch = {};
    mat4 m_cam_rot = {};
    std::memcpy(m_cam_rot, lac_ident_mat4, sizeof(m_cam_rot));

    // Calculate camera's rotation matrix from pitch and yaw
    lac_get_yaw_mat4(m_yaw, lac_deg_to_rad(camera_yaw));
    lac_get_pitch_mat4(m_pitch, lac_deg_to_rad(camera_pitch));
    lac_multiply_mat4(m_cam_rot, m_yaw, m_pitch);

    lac_multiply_vec4_mat4(v4_new_look_dir, v4_new_look_dir, m_cam_rot);
    v_look_dir[0] = v4_new_look_dir[0];
    v_look_dir[1] = v4_new_look_dir[1];
    v_look_dir[2] = v4_new_look_dir[2];
    lac_normalize_vec3(v_look_dir, v_look_dir);

    // New look direction is the rotated look vector + camera's current position
    lac_add_vec3(v3_new_look_dir, v_eye, v_look_dir);
    lac_get_point_at_mat4(m_point_at, v_eye, v3_new_look_dir, v_up);
    lac_invert_mat4(m_view->data(), m_point_at);
}

/**
 * Frustum:
 *
 *       w_half
 *  v_B _______________ v_C
 *      \      ^      /
 *       \     |     /
 *        \    |    /
 *    hyp  \   |   /
 *          \  |  /
 *           \ | /
 *            \|/
 *        v_eye / v_A
 */
CullingFrustum Camera::get_frustum_coords(uint8_t distance)
{
    GameState &game = GameState::get_instance();
    ssize_t chunk_size = game.chunk_size;

    const float zfar = chunk_size * distance;
    const float w_half = std::tanf(lac_deg_to_rad(game.fov / 2.0f)) * zfar;

    vec2 tmp = {};
    vec2 v_A = { -v_eye[0], v_eye[1] };

    vec2 v_LD = { v_look_dir[0], -v_look_dir[1] };
    // TODO: Why does this need normalizing if we're doing it in calculate_view_matrix?
    lac_normalize_vec2(v_LD, v_LD);
    lac_multiply_vec2(v_LD, v_LD, zfar);
    lac_add_vec2(v_LD, v_LD, v_A);

    lac_subtract_vec2(tmp, v_A, v_LD);

    vec2 v_B = { -tmp[1], tmp[0] };
    lac_normalize_vec2(v_B, v_B);
    lac_multiply_vec2(v_B, v_B, w_half);
    lac_add_vec2(v_B, v_B, v_LD);

    vec2 v_C = { tmp[1], -tmp[0] };
    lac_normalize_vec2(v_C, v_C);
    lac_multiply_vec2(v_C, v_C, w_half);
    lac_add_vec2(v_C, v_C, v_LD);

    return CullingFrustum{
        std::array<float, 2>{ v_A[0], v_A[1] },
        std::array<float, 2>{ v_B[0], v_B[1] },
        std::array<float, 2>{ v_C[0], v_C[1] }
    };
}

