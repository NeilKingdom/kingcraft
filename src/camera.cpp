#include <iostream>
#include <memory>
#include <array>
#include <cstring>

#include <transforms.h>

#include "../include/camera.hpp"

Camera::Camera() :
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

Camera::~Camera() 
{}

void Camera::updateVelocity(float playerSpeed)
{
    // Calculate forward camera velocity
    lac_multiply_vec3(&vFwdVel, vLookDir, playerSpeed); 
    // Calculate right camera velocity
    lac_calc_cross_prod(const_cast<vec3*>(&vRight), vLookDir, vUp);
    lac_multiply_vec3(&vRightVel, vRight, playerSpeed);
}

void Camera::rotateFromPointer(Display *dpy, Window win, XWindowAttributes xwa) 
{
    Window wnop;
    int x, y, inop;
    float centerX, centerY, normDx, normDy;
    mat4 mRot;

    XQueryPointer(dpy, win, &wnop, &wnop, &inop, &inop, &x, &y, (unsigned int*)&inop);
    centerX = (float)xwa.width / 2.0f;
    centerY = (float)xwa.height / 2.0f;
    normDx  = (centerX - (float)x) / (float)xwa.width;
    normDy  = (centerY - (float)y) / (float)xwa.height;

    const float cameraRoll = normDx * CAMERA_BASE_SPEED;
    const float cameraPitch = normDy * CAMERA_BASE_SPEED;

    mat4 mRoll, mPitch;
    lac_get_roll_mat4(&mRoll, lac_deg_to_rad(cameraRoll));
    lac_get_pitch_mat4(&mPitch, lac_deg_to_rad(cameraPitch));
    lac_multiply_mat4(&mRot, mRoll, mPitch);
    lac_multiply_mat4(&mCamRot, mCamRot, mRot);

    XWarpPointer(dpy, win, win, 0, 0, xwa.width, xwa.height, centerX, centerY);
}

void Camera::calculateViewMatrix() 
{
    std::memcpy(vNewLookDir, vFwd, sizeof(vNewLookDir));

    // Store vTarget as a vec4 with w component of 1
    vec4 tmpNewLookDir = { 1.0f };
    std::memcpy(tmpNewLookDir, vNewLookDir, sizeof(vNewLookDir));

    // Multiply vTarget with camera's rotation matrix
    lac_multiply_mat4_vec4(&tmpNewLookDir, mCamRot, vNewLookDir);

    // Revert back to using vec3s
    std::memcpy(vLookDir, tmpNewLookDir, sizeof(vLookDir));

    // TODO: Add comment
    lac_add_vec3(&vNewLookDir, vEye, vLookDir);
    lac_get_point_at_mat4(&mPointAt, vEye, vNewLookDir, vUp);
    // TODO: Probably more idiomatic way of doing this
    mat4 view;
    std::memcpy(view, mView.get(), sizeof(view));
    lac_invert_mat4(&view, mPointAt);
}
