#pragma once

// C++ APIs
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <chrono>
#include <algorithm>
#include <memory>
#include <vector>
#include <array>
#include <optional>
#include <tuple>

// C APIs
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cstdint>
#include <cstdint>
#include <cstddef>

// X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/Xcursor/Xcursor.h>

// OpenGL
#include <GL/glew.h> // NOTE: Must be placed before other OpenGL headers
#include <GL/gl.h>   // General OpenGL APIs
#include <GL/glx.h>  // X11-specific OpenGL APIs

// liblac
#include <transforms.h>

// Custom X cursor
struct XCursor
{
    Cursor cursor;
    Pixmap cpmap;
};

// Window data (right now only X11 is supported)
struct KCWindow
{
    Display                *dpy;    // The target monitor/display (assuming we might have multiple displays)
    Window                  win;    // The application's parent window
    XVisualInfo            *xvi;    // Struct containing additional info about the window
    XWindowAttributes       xwa;    // Struct containing the window's attributes
    XEvent                  xev;    // Stores the event type of the most recently received event
    GLXContext              glx;
    Colormap               cmap;    // Colormap for the X window
    XCursor                 cur;    // Custom cursor to replace the default one
};

enum Face : uint8_t
{
    RIGHT  = 1 << 0,
    LEFT   = 1 << 1,
    BACK   = 1 << 2,
    FRONT  = 1 << 3,
    BOTTOM = 1 << 4,
    TOP    = 1 << 5,
    ALL    = 0xFF
};
