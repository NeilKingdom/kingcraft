#include "../include/camera.hpp"

Camera::Camera() :
vEye{ 0 },
vFwd{ 0.0f, 0.0f, 1.0f },
vRight{ 1.0, 0.0f, 0.0f },
vUp{ 0.0f, 1.0f, 0.0f },
vTarget{ 0.0f, 0.0f, 1.0f }, 
mCamRot{ 0 },
mPointAt{ 0 },
mView{ 0 }
{
   std::memcpy(mCamRot, lac_ident_mat4, sizeof(mCamRot));
}

Camera::~Camera() 
{}

void Camera::updateVelocity(vec3 &vFwdVel, vec3 &vRightVel, float playerSpeed) const
{
   // Calculate forward camera velocity
   lac_multiply_vec3(&vFwdVel, vFwd, playerSpeed); 
   // Calculate right camera velocity
   lac_calc_cross_prod(&vRightVel, vFwd, vUp);
   lac_multiply_vec3(&vRightVel, vRightVel, -playerSpeed);
}

void Camera::rotateFromPointer(Display *dpy, Window win, XWindowAttributes xwa) 
{
   Window wnop;
   int x, y, inop;
   float center_x, center_y, norm_dx, norm_dy;

   XQueryPointer(dpy, win, &wnop, &wnop, &inop, &inop, &x, &y, (unsigned int*)&inop);
   center_x = (float)xwa.width / 2.0f;
   center_y = (float)xwa.height / 2.0f;
   norm_dx  = (center_x - (float)x) / (float)xwa.width;
   norm_dy  = (center_y - (float)y) / (float)xwa.height;

   float camera_pitch = norm_dx * CAMERA_BASE_SPEED;
   float camera_roll = norm_dy * CAMERA_BASE_SPEED;
   lac_get_rotation_mat4(&mCamRot, lac_deg_to_rad(camera_roll), lac_deg_to_rad(camera_pitch), 0.0f);

   XWarpPointer(dpy, win, win, 0, 0, xwa.width, xwa.height, center_x, center_y);
}

void Camera::calculateViewMatrix() 
{
   // Store v_fwd as a vec4 with w component of 1
   vec4 tmp_v_fwd = { 1 };
   std::memcpy(tmp_v_fwd, vFwd, sizeof(vFwd));
   // Multiply v_fwd with camera's rotation matrix
   lac_multiply_mat4_vec4(&tmp_v_fwd, mCamRot, vFwd);
   // Revert back to using vec3s
   std::memcpy(vFwd, tmp_v_fwd, sizeof(vFwd));
   
   lac_add_vec3(&vTarget, vEye, vFwd);
   lac_get_point_at_mat4(&mPointAt, vEye, vTarget, vUp);
   lac_invert_mat4(&mView, mPointAt);
}
