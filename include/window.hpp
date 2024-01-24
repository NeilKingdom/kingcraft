#pragma once 

#include "common.hpp"
#include "constants.hpp"
#include "camera.hpp"

/* Only transforms.h is required, but LSP gets confused */
#include <vecmath.h> 
#include <matmath.h>
#include <transforms.h>

static uint16_t keyMask = 0;

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

static float fov = lac_deg_to_rad(90.0f);
static float znear = 1.0f;
static float zfar = 1000.0f;

// X11 data
typedef struct {
    Display                *dpy;    // The target monitor/display (assuming we might have multiple displays)
    Window                  win;    // The application's parent window
    XVisualInfo            *xvi;    // Struct containing additional info about the window
    XWindowAttributes       xwa;    // Struct containing the window's attributes
    XEvent                  xev;    // Stores the event type of the most recently received event
    Colormap               cmap;    // Colormap for the X window 
    GLXContext              glx;    // The OpenGL context for X11
} xObjects;

#include "impl_imgui.hpp"

// OpenGL data
typedef struct {
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    unsigned int shader;
} glObjects;

struct Mvp {
    mat4 mModel;
    std::shared_ptr<std::array<float, 16>> mView;
    mat4 mProj;

    Mvp(Camera &camera) : 
        mModel{ 0 },
        mView(camera.mView),
        mProj{ 0 }
    {}
};

typedef GLXContext (*glXCreateContextAttribsARBProc)(
    Display *dpy, 
    GLXFBConfig fbConf, 
    GLXContext glx, 
    Bool isFwdCompat, 
    const int *glxAttribs
);

// Forward function declarations

void calculateFrameRate(int &fps, int &fpsInc, std::chrono::steady_clock::time_point &timePrev);
unsigned compileShader(const unsigned type, const std::string source);
unsigned createShader(const std::string vertexShader, const std::string fragmentShader);
bool isGLXExtensionSupported(const char *extList, const char *extName);
GLXFBConfig createXWindow(xObjects &xObjs, const std::string winName, const size_t winWidth = 1920, const size_t winHeight = 1080);
void createOpenGLContext(xObjects &xObjs, GLXFBConfig &bestFbConfig);
void processEvents(xObjects &xObjs, Camera &camera, bool &getPtrLocation, const float playerSpeed);
void renderFrame(xObjects &xObjs, glObjects &glObjs, Mvp &mvp, Camera &camera, const size_t indicesSize, Display *imGuiDpy = nullptr, Window imGuiWin = None);
