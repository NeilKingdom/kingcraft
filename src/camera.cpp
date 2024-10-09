#include "camera.hpp"

/**
 * @brief Default constructor for the Camera class.
 * @since 03-02-2024
 */
Camera::Camera() :
    cameraRoll(0),
    cameraPitch(0),
    vEye{ 0 },
    vLookDir{ 0 },
    vNewLookDir{ 0 },
    vFwdVel{ 0 },
    vRightVel{ 0 },
    mCamRot{ 0 },
    mPointAt{ 0 },
    mView(std::make_shared<std::array<float, 16>>())
{
    std::fill(mView->begin(), mView->end(), 0.0f);
    std::memcpy(mCamRot, lac_ident_mat4, sizeof(mCamRot));
}

/**
 * @brief Update Camera's velocity vector given the scalar value of playerSpeed.
 * @since 03-02-2024
 * @param[in] playerSpeed The scalar value used to multiply the Camera's velocity vector
 */
void Camera::updateVelocity(const float playerSpeed)
{
    // Calculate forward camera velocity
    lac_multiply_vec3(&vFwdVel, vLookDir, playerSpeed);
    // Calculate right camera velocity
    lac_calc_cross_prod(const_cast<vec3*>(&vRight), vLookDir, vUp);

    lac_normalize_vec3(const_cast<vec3*>(&vRight), vRight);
    lac_multiply_vec3(&vRightVel, vRight, playerSpeed);
}

/**
 * @brief Update the Camera's rotation matrix given the position of the mouse pointer on screen.
 * @since 03-02-2024
 */
void Camera::updateRotationFromPointer(const xObjects &xObjs)
{
    Window wnop;
    int x, y, inop;
    float centerX, centerY, normDx, normDy;

    XQueryPointer(xObjs.dpy, xObjs.win, &wnop, &wnop, &inop, &inop, &x, &y, (unsigned*)&inop);
    centerX = (float)xObjs.xwa.width / 2.0f;
    centerY = (float)xObjs.xwa.height / 2.0f;
    normDx  = (centerX - (float)x) / (float)xObjs.xwa.width;
    normDy  = (centerY - (float)y) / (float)xObjs.xwa.height;

    cameraRoll += normDx * Camera::CAMERA_BASE_SPEED;
    cameraPitch += normDy * Camera::CAMERA_BASE_SPEED;
    cameraPitch = std::clamp(cameraPitch, -89.0f, 89.0f);

    XWarpPointer(xObjs.dpy, xObjs.win, xObjs.win, 0, 0, xObjs.xwa.width, xObjs.xwa.height, centerX, centerY);
}

void Camera::calculateViewMatrix()
{
    // Rotation matrix from pointer location
    mat4 mRoll, mPitch;
    lac_get_roll_mat4(&mRoll, lac_deg_to_rad(cameraRoll));
    lac_get_pitch_mat4(&mPitch, lac_deg_to_rad(cameraPitch));
    lac_multiply_mat4(&mCamRot, mRoll, mPitch);

    // Initialize vNewlookDir to vFwd
    std::memcpy(vNewLookDir, vFwd, sizeof(vNewLookDir));

    // Store vNewLookDir as a vec4 with w component of 1
    vec4 tmpNewLookDir = { 1.0f };
    std::memcpy(tmpNewLookDir, vNewLookDir, sizeof(vNewLookDir));

    // Multiply vNewLookDir with camera's rotation matrix
    lac_multiply_vec4_mat4(&tmpNewLookDir, vNewLookDir, mCamRot);

    // Revert back to using vec3
    std::memcpy(vLookDir, tmpNewLookDir, sizeof(vLookDir));

    // vNewLookDir is the rotated look vector + camera's current position
    lac_add_vec3(&vNewLookDir, vEye, vLookDir);
    lac_get_point_at_mat4(&mPointAt, vEye, vNewLookDir, vUp);
    lac_invert_mat4(reinterpret_cast<mat4*>(mView->data()), mPointAt);
}
