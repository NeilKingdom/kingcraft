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
    v_look_dir(KC::v_fwd),
    camera_yaw(0.0f),
    camera_pitch(0.0f)
{
    m_view = std::make_shared<Mat4_t>(qm_m4_ident);
}

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
    camera_yaw += norm_dx * 180.0f * KC::CAMERA_SPEED_FACTOR;
    camera_pitch -= norm_dy * 180.0f * KC::CAMERA_SPEED_FACTOR;
    camera_pitch = std::clamp(camera_pitch, -89.0f, 89.0f);

    // Warp cursor back to center of screen
    XWarpPointer(win.dpy, None, win.win, 0, 0, 0, 0, (int)center_x, (int)center_y);
}

bool Camera::is_chunk_in_visible_radius(const Vec3_t chunk_location) const
{
    Settings &settings = Settings::get_instance();

    float a = chunk_location.x - std::floorf(v_eye.x / KC::CHUNK_SIZE);
    float b = chunk_location.y - std::floorf(v_eye.y / KC::CHUNK_SIZE);
    float c = std::sqrtf((a * a) + (b * b));

    return c < settings.render_distance;
}


std::optional<Block> Camera::cast_ray(const uint8_t n_iters) const
{
    ChunkManager& chunk_mgr = ChunkManager::get_instance();

    Vec3_t v_step = this->v_look_dir;
    Vec3_t v_ray = this->v_eye;

    for (uint8_t i = 0; i < n_iters; ++i)
    {
        v_ray = qm_v3_add(v_ray, v_step);

        float wx = std::floorf(v_ray.x);
        float wy = std::floorf(v_ray.y);
        float wz = std::floorf(v_ray.z);

        int cx = std::floorf(wx / KC::CHUNK_SIZE);
        int cy = std::floorf(wy / KC::CHUNK_SIZE);
        int cz = std::floorf(wz / KC::CHUNK_SIZE);

        int bx = (((int)wx % KC::CHUNK_SIZE) + KC::CHUNK_SIZE) % KC::CHUNK_SIZE;
        int by = (((int)wy % KC::CHUNK_SIZE) + KC::CHUNK_SIZE) % KC::CHUNK_SIZE;
        int bz = (((int)wz % KC::CHUNK_SIZE) + KC::CHUNK_SIZE) % KC::CHUNK_SIZE;

        auto chunk = chunk_mgr.GCL.find((Vec3_t){ .v = { (float)cx, (float)cy, (float)cz }});
        if (chunk == nullptr)
        {
            std::cout << "nullptr" << std::endl;
            return std::nullopt;
        }

        std::cout << "Step " << i + 1 << std::endl;
        std::cout << "(" << v_step.x << "," << v_step.y << "," << v_step.z << ")" << std::endl;
        std::cout << "(" << v_ray.x << "," << v_ray.y << "," << v_ray.z << ")" << std::endl;
        std::cout << "(" << wx << "," << wy << "," << wz << ")" << std::endl;
        std::cout << "(" << cx << "," << cy << "," << cz << ")" << std::endl;
        std::cout << "(" << bx << "," << by << "," << bz << ")" << std::endl;

        Block& block = chunk->blocks[bz][by][bx];
        if (block.type != BlockType::AIR)
        {
            std::cout << "Block type: " << (int)block.type << std::endl;
            block.type = BlockType::STONE;
            chunk->update_mesh();
            return block;
        }
    }

    return std::nullopt;
}
