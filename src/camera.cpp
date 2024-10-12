#include "camera.hpp"

/**
 * @brief Default constructor for the Camera class.
 * @since 03-02-2024
 */
Camera::Camera(
    const float fov,
    const float aspect,
    const float znear,
    const float zfar
) :
    fov(fov),
    aspect(aspect),
    znear(znear),
    zfar(zfar),
    camera_yaw(0),
    camera_pitch(0),
    v_eye{ 0 },
    v_look_dir{ 0 },
    v_new_look_dir{ 0 },
    v_fwd_vel{ 0 },
    v_right_vel{ 0 },
    m_cam_rot{ 0 },
    m_point_at{ 0 },
    m_view(std::make_shared<std::array<float, 16>>())
{
    std::fill(m_view->begin(), m_view->end(), 0.0f);
    std::memcpy(m_cam_rot, lac_ident_mat4, sizeof(m_cam_rot));
}

/**
 * @brief Update Camera's velocity vector given the scalar value of __player_speed__.
 * @since 03-02-2024
 * @param[in] playerSpeed The scalar value used to multiply the Camera's velocity vector
 */
void Camera::update_velocity(const float player_speed)
{
    // Calculate forward camera velocity
    lac_multiply_vec3(&v_fwd_vel, v_look_dir, player_speed);
    // Calculate right camera velocity
    lac_calc_cross_prod(const_cast<vec3*>(&v_right), v_look_dir, v_up);

    lac_normalize_vec3(const_cast<vec3*>(&v_right), v_right);
    lac_multiply_vec3(&v_right_vel, v_right, player_speed);
}

/**
 * @brief Update the Camera's rotation matrix given the position of the mouse pointer on screen.
 * @since 03-02-2024
 */
void Camera::update_rotation_from_pointer(const XObjects &x_objs)
{
    Window wnop;
    int x, y, inop;
    float center_x, center_y, norm_dx, norm_dy;

    XQueryPointer(x_objs.dpy, x_objs.win, &wnop, &wnop, &inop, &inop, &x, &y, (unsigned*)&inop);
    center_x = (float)x_objs.xwa.width / 2.0f;
    center_y = (float)x_objs.xwa.height / 2.0f;
    norm_dx  = (center_x - (float)x) / (float)x_objs.xwa.width;
    norm_dy  = (center_y - (float)y) / (float)x_objs.xwa.height;

    camera_yaw += norm_dx * Camera::CAMERA_BASE_SPEED;
    camera_pitch += norm_dy * Camera::CAMERA_BASE_SPEED;
    camera_pitch = std::clamp(camera_pitch, -89.0f, 89.0f);

    XWarpPointer(x_objs.dpy, x_objs.win, x_objs.win, 0, 0, x_objs.xwa.width, x_objs.xwa.height, center_x, center_y);
}

void Camera::calculate_view_matrix()
{
    // Rotation matrix from pointer location
    mat4 m_yaw, m_pitch;
    // TODO: Should be yaw
    lac_get_roll_mat4(&m_yaw, lac_deg_to_rad(camera_yaw));
    lac_get_pitch_mat4(&m_pitch, lac_deg_to_rad(camera_pitch));
    lac_multiply_mat4(&m_cam_rot, m_yaw, m_pitch);

    // Initialize vNewlookDir to vFwd
    std::memcpy(v_new_look_dir, v_fwd, sizeof(v_new_look_dir));

    // Store vNewLookDir as a vec4 with w component of 1
    vec4 tmp_new_look_dir = { 1.0f };
    std::memcpy(tmp_new_look_dir, v_new_look_dir, sizeof(v_new_look_dir));

    // Multiply vNewLookDir with camera's rotation matrix
    lac_multiply_vec4_mat4(&tmp_new_look_dir, v_new_look_dir, m_cam_rot);

    // Revert back to using vec3
    std::memcpy(v_look_dir, tmp_new_look_dir, sizeof(v_look_dir));

    // vNewLookDir is the rotated look vector + camera's current position
    lac_add_vec3(&v_new_look_dir, v_eye, v_look_dir);
    lac_get_point_at_mat4(&m_point_at, v_eye, v_new_look_dir, v_up);
    lac_invert_mat4(reinterpret_cast<mat4*>(m_view->data()), m_point_at);
}
