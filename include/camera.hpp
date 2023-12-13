#pragma once

#include <cstring>

#include <X11/Xlib.h>

#include <vecmath.h>
#include <matmath.h>
#include <transforms.h>

class Camera
{
public:
    const float CAMERA_BASE_SPEED = 20;

    vec3 vEye;  // Camera's origin point
    mat4 mView; // View matrix
    vec3 vUp;   // Orthogonal to vLookDir and vRight     

    Camera();
    ~Camera();

    void updateVelocity(vec3 &vFwdVel, vec3 &vRightVel, float playerSpeed);
    void rotateFromPointer(Display *dpy, Window win, XWindowAttributes xwa);
    void calculateViewMatrix();

private:
    vec3 vLookDir;    // The direction that the camera is facing
    vec3 vRight;      // Orthogonal to vLookDir and vUp

    mat4 mPointAt;    // The point-at matrix which becomes the view matrix after inversion
    mat4 mCamRot;     // Rotation matrix used for transforming vTarget
};
