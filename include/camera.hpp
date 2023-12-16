#pragma once

#include <cstring>
#include <memory>

#include <X11/Xlib.h>

#include <vecmath.h>
#include <matmath.h>
#include <transforms.h>

class Camera
{
public:
    const float CAMERA_BASE_SPEED = 20;

    const vec3 vFwd = { 0.0f, 0.0f, 1.0f };
    const vec3 vRight = { 1.0, 0.0f, 0.0f };
    const vec3 vUp = { 0.0f, 1.0f, 0.0f };

    vec3 vEye;                                      // Camera's origin point
    vec3 vFwdVel;                                   // Forward velocity vector 
    vec3 vRightVel;                                 // Right velocity vector
    std::shared_ptr<std::array<float, 16>> mView;   // View matrix

    // Constructor/Destructor
    Camera();
    ~Camera();

    // Member functions
    void updateVelocity(float playerSpeed);
    void rotateFromPointer(Display *dpy, Window win, XWindowAttributes xwa);
    void calculateViewMatrix();

private:
    vec3 vLookDir;    // The direction that the camera is facing on the current frame
    vec3 vNewLookDir; // The direction that the camera will be facing on the next frame

    mat4 mPointAt;    // The point-at matrix which becomes the view matrix after inversion
    mat4 mCamRot;     // Rotation matrix used for transforming vLookDir
};
