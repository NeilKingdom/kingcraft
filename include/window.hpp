#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "camera.hpp"
#include "block.hpp"
#include "shader_program.hpp"
#include "chunk.hpp"
#include "skybox.hpp"
#include "chunk_factory.hpp"

struct KCShaders
{
    ShaderProgram block;
    ShaderProgram skybox;
};

static bool query_pointer_location = true;

enum KeyLabel : uint64_t
{
    KEY_MOVE_FORWARD  = (1 << 0),
    KEY_MOVE_BACKWARD = (1 << 1),
    KEY_MOVE_LEFT     = (1 << 2),
    KEY_MOVE_RIGHT    = (1 << 3),
    KEY_MOVE_UP       = (1 << 4),
    KEY_MOVE_DOWN     = (1 << 5),
    KEY_EXIT_GAME     = (1 << 6),
};

static uint64_t key_mask = 0;
static auto key_binds = std::map<KeySym, KeyLabel>{
    { XK_w,             KeyLabel::KEY_MOVE_FORWARD  },
    { XK_s,             KeyLabel::KEY_MOVE_BACKWARD },
    { XK_a,             KeyLabel::KEY_MOVE_LEFT     },
    { XK_d,             KeyLabel::KEY_MOVE_RIGHT    },
    { XK_space,         KeyLabel::KEY_MOVE_UP       },
    { XK_BackSpace,     KeyLabel::KEY_MOVE_DOWN     },
    { XK_q,             KeyLabel::KEY_EXIT_GAME     }
};

typedef GLXContext (*glXCreateContextAttribsARBProc)(
    Display *dpy,
    GLXFBConfig fb_config,
    GLXContext glx,
    Bool is_fwd_compat,
    const int *glx_attribs
);

// TODO: Move into separate header?
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
GLXContext      create_opengl_context(KCWindow &win, const GLXFBConfig &fb_config);
void            process_events(KCWindow &win, Camera &camera);
void            render_frame(const KCWindow &win, Camera &camera, Mvp &mvp, const GameState &game, KCShaders &shaders, const std::set<std::shared_ptr<Chunk>> &chunks, SkyBox &skybox);
