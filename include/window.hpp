#pragma once

#include "common.hpp"
#include "shader_program.hpp"

struct KCShaders
{
    ShaderProgram block;
    ShaderProgram skybox;
};

static bool query_pointer_location = true;
static uint64_t key_mask = 0;

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

// Forward function declarations

void        calculate_frame_rate(int &fps, int &frames_elapsed, std::chrono::steady_clock::time_point &since);
GLXFBConfig create_window(KCWindow &win, const std::string win_name, const size_t win_width, const size_t win_height);
GLXContext  create_opengl_context(KCWindow &win, const GLXFBConfig &fb_config);
