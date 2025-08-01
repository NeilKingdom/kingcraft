#pragma once

// C++ APIs
#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include <algorithm>
#include <array>
#include <vector>
#include <unordered_set>
#include <set>
#include <map>
#include <queue>
#include <tuple>
#include <iterator>
#include <optional>
#include <limits>
#include <filesystem>
#include <chrono>
#include <random>
#include <thread>
#include <atomic>

// C APIs
#include <cmath>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cstdint>
#include <cstddef>

// X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/cursorfont.h>

// OpenGL
#include <GL/glew.h> // NOTE: Must be placed before other OpenGL headers
#include <GL/gl.h>   // General OpenGL APIs
#include <GL/glx.h>  // X11-specific OpenGL APIs

// liblac
#include <transforms.h>

#define SET_BIT(mask, bit)    ((mask) |= (bit))
#define UNSET_BIT(mask, bit)  ((mask) &= (~bit))
#define IS_BIT_SET(mask, bit) (((mask) & (bit)) == (bit))
#define TOGGLE_BIT(mask, bit) ((mask) ^= (bit))

// Window data (right now only X11 is supported)
struct KCWindow
{
    Display                *dpy;    // The target monitor/display (assuming we might have multiple displays)
    Window                  win;    // The application's parent window
    XVisualInfo            *xvi;    // Struct containing additional info about the window
    XWindowAttributes       xwa;    // Struct containing the window's attributes
    XEvent                  xev;    // Stores the event type of the most recently received event
    GLXContext              glx;    // OpenGL context
    Colormap               cmap;    // Colormap for the X window

    // Custom cursor (right now only X11 is supported)
    struct KCCursor
    {
        Cursor cursor;
        Pixmap cpmap;
    } cur;
};

enum BlockFace : uint8_t
{
    RIGHT  = (1 << 0),
    LEFT   = (1 << 1),
    BACK   = (1 << 2),
    FRONT  = (1 << 3),
    BOTTOM = (1 << 4),
    TOP    = (1 << 5),
    ALL    = (RIGHT | LEFT | BACK | FRONT | BOTTOM | TOP)
};

typedef unsigned ID;
extern std::atomic<unsigned> fps;
extern float avg_chunk_proc_time;
