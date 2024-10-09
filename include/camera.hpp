#pragma once

#include "common.hpp"

class Camera
{
public:
    static constexpr float CAMERA_BASE_SPEED = 20.0f;

    const vec3 vFwd   = { 0.0f, 0.0f, 1.0f };
    const vec3 vRight = { 1.0f, 0.0f, 0.0f };
    const vec3 vUp    = { 0.0f, 1.0f, 0.0f };

    vec3 vEye;                                      // Camera's origin point
    vec3 vFwdVel;                                   // Forward velocity vector
    vec3 vRightVel;                                 // Right velocity vector
    std::shared_ptr<std::array<float, 16>> mView;   // View matrix

    // Special member functions
    Camera();
    ~Camera() = default;

    // General
    void updateVelocity(const float playerSpeed);
    void updateRotationFromPointer(const xObjects &xObjs);
    void calculateViewMatrix();

private:
    float cameraRoll;
    float cameraPitch;

    vec3 vLookDir;    // The direction that the camera is facing on the current frame
    vec3 vNewLookDir; // The direction that the camera will be facing on the next frame

    mat4 mPointAt;    // The point-at matrix which becomes the view matrix after inversion
    mat4 mCamRot;     // Rotation matrix used for transforming vLookDir
};
