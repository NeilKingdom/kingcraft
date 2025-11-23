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
    v_eye{},
    m_view(std::make_shared<std::array<float, 16>>()),
    camera_yaw(0.0f),
    camera_pitch(0.0f)
{
    std::memcpy(v_look_dir, KC::v_fwd, sizeof(vec3));
}

/**
 * @brief Calculates the view matrix based on the look direction of the camera.
 * @since 02-03-2024
 */
void Camera::calculate_view_matrix()
{
    vec3 v3_new_look_dir = {};
    vec4 v4_new_look_dir = { KC::v_fwd[0], KC::v_fwd[1], KC::v_fwd[2], 1.0f };
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
    lac_get_point_at_mat4(m_point_at, v_eye, v3_new_look_dir, KC::v_up);
    lac_invert_mat4(m_view->data(), m_point_at);
}

/**
 * @brief Update the Camera's rotation matrix given the position of the mouse/pointer on screen.
 * @since 02-03-2024
 * @param[in] win A reference to the application's window
 * @param[in] pointer_pos The current (x, y) position of the mouse pointer relative to the parent window
 */
void Camera::update_rotation_from_pointer(const KCWindow &win)
{
    // Cursor position
    float ptr_x = win.xev.xmotion.x;
    float ptr_y = win.xev.xmotion.y;

    // Screen width/height
    float screen_w = win.xwa.width;
    float screen_h = win.xwa.height;

    // Screen center
    float center_x = screen_w / 2.0f;
    float center_y = screen_h / 2.0f;

    // Normalized mouse pointer position deltas
    float norm_dx = (center_x - ptr_x) / screen_w;
    float norm_dy = (center_y - ptr_y) / screen_h;

    // TODO: Multiply by delta time to smooth
    // Convert from pixel space to degrees
    camera_yaw += norm_dx * 180.0f * KC::CAMERA_SPEED_FACTOR;
    camera_pitch += norm_dy * 180.0f * KC::CAMERA_SPEED_FACTOR;
    camera_pitch = std::clamp(camera_pitch, -89.0f, 89.0f);

    // Warp cursor back to center of screen
    XWarpPointer(win.dpy, None, win.win, 0, 0, 0, 0, (int)center_x, (int)center_y);
}

//std::optional<Block> Camera::cast_ray(
//    const std::vector<std::shared_ptr<Chunk>> &chunks,
//    const unsigned n_iters
//) const
//{
//    GameState &game = GameState::get_instance();
//    ssize_t chunk_size = game.chunk_size;
//
//    vec3 v_ray = { v_look_dir[0], v_look_dir[1], v_look_dir[2] };
//
//    vec3 chunk_location = {
//        std::floorf(v_eye[0] / chunk_size),
//        std::floorf(v_eye[1] / chunk_size),
//        std::floorf(v_eye[2] / chunk_size)
//    };
//
//    for (unsigned i = 0; i < n_iters; ++i)
//    {
//        // March ray
//        lac_normalize_vec3(v_ray, v_ray);
//        for (unsigned j = 0; j < n_iters; ++j)
//        {
//            lac_add_vec3(v_ray, v_ray, v_ray);
//            v_ray[0] = std::floorf(v_ray[0]);
//            v_ray[1] = std::floorf(v_ray[1]);
//            v_ray[2] = std::floorf(v_ray[2]);
//        }
//
//        // Check for block
//        Chunk needle = Chunk();
//        std::memcpy(needle.location, chunk_location, sizeof(vec3));
//
//        for (auto chunk : chunks)
//        {
//            if (*chunk.get() == needle)
//            {
//                Block &block = chunk.get()->blocks[v_ray[0]][v_ray[1]][v_ray[2]];
//                if (block.type != BlockType::AIR)
//                {
//                    std::cout << "Looking at "
//                        << "x: " << v_ray[0] << ", "
//                        << "y: " << v_ray[1] << ", "
//                        << "z: " << v_ray[2]
//                        << std::endl;
//                    return block;
//                }
//            }
//        }
//    }
//
//    return std::nullopt;
//}
