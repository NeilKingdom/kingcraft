#include <cstring>
#include <iostream>

#include <matmath.h>
#include <transforms.h>
#include <vecmath.h>

#include "../include/camera.hpp"

Camera::Camera() :
    vEye{ 0 },
    vLookDir{ 0 },
    vRight{ 1.0, 0.0f, 0.0f },
    vUp{ 0.0f, 1.0f, 0.0f },
    mCamRot{ 0 },
    mPointAt{ 0 },
    mView{ 0 }
{
    std::memcpy(mCamRot, lac_ident_mat4, sizeof(mCamRot));
}

Camera::~Camera() 
{}

void Camera::updateVelocity(vec3 &vFwdVel, vec3 &vRightVel, float playerSpeed)
{
    // Calculate forward camera velocity
    lac_multiply_vec3(&vFwdVel, vLookDir, playerSpeed); 
    // Calculate right camera velocity
    lac_calc_cross_prod(const_cast<vec3*>(&vRight), vLookDir, vUp);
    lac_multiply_vec3(&vRightVel, vRight, playerSpeed);
}

// TODO: Maybe add elapsed time to hopefully smooth out jumps
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
    vec3 vTarget = { 0.0f, 0.0f, 1.0f }; 

    // Store vTarget as a vec4 with w component of 1
    vec4 tmpVTarget = { 1.0f };
    std::memcpy(tmpVTarget, vTarget, sizeof(vTarget));

    // Multiply vTarget with camera's rotation matrix
    lac_multiply_mat4_vec4(&tmpVTarget, mCamRot, vTarget);

    // Revert back to using vec3s
    std::memcpy(vLookDir, tmpVTarget, sizeof(vLookDir));

    // TODO: Add comment
    lac_add_vec3(&vTarget, vEye, vLookDir);
    lac_get_point_at_mat4(&mPointAt, vEye, vTarget, vUp);
    lac_invert_mat4(&mView, mPointAt);
}
