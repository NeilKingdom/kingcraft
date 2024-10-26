#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "camera.hpp"
#include "block.hpp"
#include "shader_program.hpp"
#include "chunk.hpp"

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

typedef GLXContext (*glXCreateContextAttribsARBProc)(
    Display *dpy,
    GLXFBConfig fb_config,
    GLXContext glx,
    Bool is_fwd_compat,
    const int *glx_attribs
);

struct Mvp
{
    mat4 m_model;
    std::shared_ptr<std::array<float, 16>> m_view;
    mat4 m_proj;

    // Special member functions
    Mvp(const Camera &camera) :
        m_model{},
        m_view(camera.m_view),
        m_proj{}
    {}
};

// Forward function declarations

void            calculate_frame_rate(int &fps, int &frames_elapsed, std::chrono::steady_clock::time_point &since);
GLXFBConfig     create_window(KCWindow &win, const std::string win_name, const size_t win_width, const size_t win_height);
GLXContext      create_opengl_context(KCWindow &win, const GLXFBConfig &best_fb_config);
void            process_events(KCWindow &win, Camera &camera);
void            render_frame(const KCWindow &win, Camera &camera, Mvp &mvp, const ShaderProgram &shader, const std::vector<std::vector<std::vector<Block>>> &blocks);
