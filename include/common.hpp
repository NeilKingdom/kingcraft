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
    Colormap               cmap;    // Colormap for the X window
    GLXContext              glx;    // The OpenGL context for X11
    XCursor                 cur;    // Custom cursor to replace the default one
};
