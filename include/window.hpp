#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "impl_imgui.hpp"
#include "camera.hpp"

static uint16_t key_mask = 0;
static bool query_pointer_location = true;

#define KEY_FORWARD     (1 << 0)
#define KEY_BACKWARD    (1 << 1)
#define KEY_LEFT        (1 << 2)
#define KEY_RIGHT       (1 << 3)
#define KEY_UP          (1 << 4)
#define KEY_DOWN        (1 << 5)

#define SET_KEY(mask, key)    ((mask) |= (key))
#define UNSET_KEY(mask, key)  ((mask) &= (~key))
#define TOGGLE_KEY(mask, key) ((mask) ^= (key))
#define IS_KEY_SET(mask, key) ((((mask) & (key)) == (key)) ? true : false)

struct Mvp
{
public:
    mat4 m_model;
    std::shared_ptr<std::array<float, 16>> m_view;
    mat4 m_proj;

    Mvp(const Camera &camera) :
        m_model{},
        m_view(camera.m_view),
        m_proj{}
    {}
};

typedef GLXContext (*glXCreateContextAttribsARBProc)(
    Display *dpy,
    GLXFBConfig fb_conf,
    GLXContext glx,
    Bool is_fwd_compat,
    const int *glx_attribs
);

// Forward function declarations

void            calculate_frame_rate(int &fps, int &frames_elapsed, std::chrono::steady_clock::time_point &since);
bool            is_glx_extension_supported(const char *ext_list, const char *ext_name);
unsigned        compile_shader(const unsigned type, const std::string source);
unsigned        create_shader(const std::string vertex_shader, const std::string fragment_shader);
GLXFBConfig     create_window(XObjects &x_objs, const std::string win_name, const size_t win_width, const size_t win_height);
void            create_opengl_context(XObjects &x_objs, const GLXFBConfig &best_fb_config);
void            process_events(XObjects &x_objs, Camera &camera);
void            render_frame(const GLObjects &gl_objs, const XObjects &x_objs, Camera &camera, Mvp &mvp, const size_t indices_size);
