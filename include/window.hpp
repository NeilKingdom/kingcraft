#pragma once

#include "common.hpp"

// X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/cursorfont.h>

// Game window (right now only X11 is supported)
struct KCWindow
{
    Display                *dpy;    // The X11 connection
    Window                  win;    // The application's parent window
    XVisualInfo            *xvi;    // Struct containing additional info about the window
    XWindowAttributes       xwa;    // Struct containing the window's attributes
    XEvent                  xev;    // Stores the event type of the most recently received event
    GLXFBConfig       fb_config;    // Frame buffer configuration returned from gfx card

    struct KCCursor
    {
        Cursor cursor;
        Pixmap pixmap;
    } cur;                          // Custom cursor
};

enum KeyAction : uint64_t
{
    PLYR_FWD      = (1 << 0),
    PLYR_BACK     = (1 << 1),
    PLYR_LEFT     = (1 << 2),
    PLYR_RIGHT    = (1 << 3),
    PLYR_UP       = (1 << 4),
    PLYR_DOWN     = (1 << 5),
    EXIT_GAME     = (1 << 6),
};
static uint64_t key_mask = 0;

static auto key_binds = std::map<KeySym, KeyAction>{
    { XK_w,             KeyAction::PLYR_FWD   },
    { XK_s,             KeyAction::PLYR_BACK  },
    { XK_a,             KeyAction::PLYR_LEFT  },
    { XK_d,             KeyAction::PLYR_RIGHT },
    { XK_space,         KeyAction::PLYR_UP    },
    { XK_BackSpace,     KeyAction::PLYR_DOWN  },
    { XK_q,             KeyAction::EXIT_GAME  }
};

typedef GLXContext (*glXCreateContextAttribsARBProc)(
    Display *dpy,
    GLXFBConfig fb_config,
    GLXContext glx,
    Bool is_fwd_compat,
    const int *glx_attribs
);

// Forward function declarations

// TODO: I'd like to return window instead of passing by param
void create_window(
    KCWindow &kc_win,
    const std::string win_name,
    const size_t win_width,
    const size_t win_height
);
GLXContext create_opengl_context(KCWindow &win);
