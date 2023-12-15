#pragma once 

// C++ APIs
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <cassert>
#include <fstream>
#include <iterator>
#include <cstring>
#include <string>
#include <chrono>
#include <algorithm>
#include <iomanip>

// C APIs
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

// X11 
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

// OpenGL
#include <GL/glew.h> // NOTE: Must be placed before other OpenGL headers
#include <GL/gl.h>   // General OpenGL APIs
#include <GL/glx.h>  // X11-specific OpenGL APIs

// TODO: For testing. Remove
#include <GLFW/glfw3.h>

// My APIs
#include "../include/callbacks.hpp"
#include "../include/camera.hpp"
#include <matmath.h>
#include <vecmath.h>
#include <transforms.h>

#define APP_TITLE "KingCraft"

static uint16_t key_mask = 0;

#define KEY_FORWARD     (1 << 0)
#define KEY_BACKWARD    (1 << 1)
#define KEY_LEFT        (1 << 2)
#define KEY_RIGHT       (1 << 3) 
#define KEY_UP          (1 << 4)
#define KEY_DOWN        (1 << 5)

#define SET_KEY(mask, key)    ((mask) |= (key))
#define UNSET_KEY(mask, key)  ((mask) &= (~key))
#define TOGGLE_KEY(mask, key) ((mask) ^= (key))
#define IS_KEY_SET(mask, key) ((((mask) & (key)) == (key)) ? (true) : (false))

static float fov = lac_deg_to_rad(90.0f);
static float znear = 1.0f;
static float zfar = 1000.0f;

// X11 variables
static Display                *dpy;    // The target monitor/display (assuming we might have multiple displays)
static Window                  win;    // The application's parent window
static XVisualInfo            *xvi;    // Struct containing additional info about the window
static XWindowAttributes       xwa;    // Struct containing the window's attributes
static XEvent                  xev;    // Stores the event type of the most recently received event
static Colormap               cmap;    // Colormap for the X window 
static GLXContext              glx;    // The OpenGL context for X11

typedef struct {
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    unsigned int shader;
} glObjects;

typedef struct {
    mat4 mModel;
    mat4 *mView;
    mat4 mProj;
} Mvp;

typedef GLXContext (*glXCreateContextAttribsARBProc)(
    Display *dpy, 
    GLXFBConfig fb_conf, 
    GLXContext glx, 
    Bool is_fwd_compat, 
    const int *glx_attribs
);

// Forward function declarations
void calculateFrameRate(int &fps, int &fps_inc, std::chrono::steady_clock::time_point &time_prev);
unsigned compileShader(unsigned type, const std::string source);
unsigned createShader(const std::string vertexShader, const std::string fragmentShader);
bool isGLXExtensionSupported(const char *extList, const char *extName);
GLXFBConfig createXWindow();
void createOpenGLContext(GLXFBConfig &bestFbConfig);
void processEvents(Camera &camera, bool &getPtrLocation, float playerSpeed);
void renderFrame(Mvp &mvp, glObjects &objs, Camera &camera, size_t indicesSize);
