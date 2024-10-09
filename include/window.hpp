#pragma once

#include "common.hpp"
#include "game_state.hpp"
#include "constants.hpp"
#include "impl_imgui.hpp"
#include "camera.hpp"

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

struct Mvp {
    mat4 mModel;
    std::shared_ptr<std::array<float, 16>> mView;
    mat4 mProj;

    Mvp(const Camera &camera) :
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

void            calculateFrameRate(int &fps, int &fpsInc, std::chrono::steady_clock::time_point &timePrev);
bool            isGLXExtensionSupported(const char *extList, const char *extName);
unsigned        compileShader(const unsigned type, const std::string source);
unsigned        createShader(const std::string vertexShader, const std::string fragmentShader);
GLXFBConfig     createXWindow(xObjects &xObjs, const std::string winName, const size_t winWidth = 1920, const size_t winHeight = 1080);
void            createOpenGLContext(xObjects &xObjs, const GLXFBConfig &bestFbConfig);
void            processEvents(const GameState &state, xObjects &xObjs, Camera &camera, bool &getPtrLocation);
void            renderFrame(GameState &state, const glObjects &glObjs, const xObjects &xObjs, Camera &camera, Mvp &mvp, const size_t indicesSize);
