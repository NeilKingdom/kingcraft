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
    // TODO: Temporary debug height
    v_eye{ .v = { 0.0f, 0.0f, KC::CHUNK_SIZE * 10 }},
    v_look_dir(KC::v_fwd),
    m_view(std::make_shared<Mat4_t>(qm_m4_ident)),
    camera_yaw(0.0f),
    camera_pitch(0.0f)
{}

/**
 * @brief Calculates the view matrix based on the look direction of the camera.
 * @since 02-03-2024
 */
void Camera::calculate_view_matrix()
{
    // Calculate rotation matrix from yaw & pitch
    Mat4_t m_yaw = qm_m4_yaw(qm_deg_to_rad(this->camera_yaw));
    Mat4_t m_pitch = qm_m4_pitch(qm_deg_to_rad(this->camera_pitch));
    Mat4_t m_cam_rot = qm_m4_mul(m_yaw, m_pitch);

    // Get look direction vector
    Vec4_t v4_look_dir = { .v = { KC::v_fwd.x, KC::v_fwd.y, KC::v_fwd.z, 0.0f }};
    v4_look_dir = qm_m4_v4_mul(m_cam_rot, v4_look_dir);

    this->v_look_dir = qm_v3_norm((Vec3_t){ .v = {
        v4_look_dir.x, v4_look_dir.y, v4_look_dir.z
    }});

    // Must add v_eye to direction vector to get true target
    Vec3_t target = qm_v3_add(this->v_eye, this->v_look_dir);
    *this->m_view = qm_m4_lookat(this->v_eye, target, KC::v_up);
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
    this->camera_yaw   += norm_dx * 180.0f * KC::CAMERA_SPEED_FACTOR;
    this->camera_pitch -= norm_dy * 180.0f * KC::CAMERA_SPEED_FACTOR;
    this->camera_pitch  = std::clamp(camera_pitch, -89.0f, 89.0f);

    // Warp cursor back to center of screen
    XWarpPointer(win.dpy, None, win.win, 0, 0, 0, 0, (int)center_x, (int)center_y);
}

bool Camera::is_chunk_in_visible_radius(const Vec3_t chunk_location) const
{
    Settings &settings = Settings::get_instance();

    float a = chunk_location.x - std::floorf(this->v_eye.x / KC::CHUNK_SIZE);
    float b = chunk_location.y - std::floorf(this->v_eye.y / KC::CHUNK_SIZE);
    float c = std::sqrtf((a * a) + (b * b));

    return c < settings.render_distance;
}


std::optional<Block> Camera::cast_ray(const uint8_t n_iters) const
{
    return std::nullopt;
}
