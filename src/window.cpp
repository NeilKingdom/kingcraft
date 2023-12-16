/**
 * Reference: https://www.khronos.org/opengl/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX)
 */

// C++ APIs
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <array>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <cassert>

// X11 
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

// OpenGL
#include <GL/glew.h> // NOTE: Must be placed before other OpenGL headers
#include <GL/gl.h>   // General OpenGL APIs
#include <GL/glx.h>  // X11-specific OpenGL APIs

// My APIs
#include <transforms.h>

#include "../include/camera.hpp"
#include "../include/constants.hpp"
#include "../include/window.hpp"

void calculateFrameRate(int &fps, int &fpsInc, std::chrono::steady_clock::time_point &timePrev)
{
    using namespace std::chrono;

    auto timeNow = steady_clock::now();
    auto timeElapsed = duration_cast<nanoseconds>(timeNow - timePrev).count();
    ++fpsInc; // Increment each frame

    if (timeElapsed > SEC_AS_NANO) // Wait until at least a second has elapsed
    {
        timePrev = timeNow;
        fps = fpsInc;
        fpsInc = 0;

        std::cout << "FPS: " << fps << std::endl;
    }
}

unsigned compileShader(unsigned type, const std::string source)
{
    unsigned id = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result); 
    if (result == GL_FALSE) 
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        std::string message;
        glGetShaderInfoLog(id, length, &length, const_cast<char*>(message.c_str()));
        std::cerr << "Failed to compile " 
            << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
            << " shader: " << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

unsigned createShader(const std::string vertexShader, const std::string fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

bool isGLXExtensionSupported(const char *extList, const char *extName) 
{
    const char *start, *where, *terminator;
    where = strchr(extName, ' ');
    start = extList;

    if (where || *start == '\0') return false;

    while (true) 
    {
        where = strstr(start, extName);
        if (!where) {
            return false;
        } 

        terminator = where + strlen(extName);
        if (where == start || *(where - 1) == ' ') 
        {
            if (*terminator == ' ' || *terminator == '\0') 
            {
                return true;
            }
        }

        start = terminator;
    }
}

GLXFBConfig createXWindow(const std::string winName, size_t winWidth, size_t winHeight)
{
    // Open the X11 display
    dpy = XOpenDisplay(NULL); 
    if (dpy == NULL) 
    {
        std::cerr << "Cannot connect to X server" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Get OpenGL version
    int vMajor, vMinor;
    glXQueryVersion(dpy, &vMajor, &vMinor);

    // Specify attributes for the frame buffer configuration
    int fbAttribs[] = {
        GLX_X_RENDERABLE,    True,
        GLX_DRAWABLE_TYPE,   GLX_WINDOW_BIT,
        GLX_RENDER_TYPE,     GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE,   GLX_TRUE_COLOR,
        GLX_RED_SIZE,        8,
        GLX_GREEN_SIZE,      8,
        GLX_BLUE_SIZE,       8,
        GLX_ALPHA_SIZE,      8,
        GLX_DEPTH_SIZE,      24,
        GLX_STENCIL_SIZE,    8,
        /*
         NOTE: The buffer swap for double buffering is synchronized with your monitor's
         vertical refresh rate (v-sync). Disabling double buffering effectively
         unlocks the framerate as the buffer swaps no longer need to align with v-sync.
       */
        GLX_DOUBLEBUFFER,    True,
        None
    };

    /*** Takes in our attributes and returns a list of frame buffers loosely matching the criteria ***/
    
    int fbCount;
    GLXFBConfig *fbConfig = glXChooseFBConfig(dpy, win, fbAttribs, &fbCount);
    if (fbConfig == NULL)
    {
        std::cerr << "Failed to retrieve framebuffer configuration" << std::endl;
        XCloseDisplay(dpy);
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG 
    std::cout << "Found " << fbCount << " matching configs" << std::endl;
#endif

    /*** Pick the XVisualInfo struct with the most samples per-pixel from the frame buffer list ***/
    
    int bestFbIdx = 0; 
    int bestSpp = 0;  

    for (int i = 0; i < fbCount; ++i) 
    {
        xvi = glXGetVisualFromFBConfig(dpy, fbConfig[i]);
        if (xvi != NULL) 
        {
            int sampBuf, samples;
            glXGetFBConfigAttrib(dpy, fbConfig[i], GLX_SAMPLE_BUFFERS, &sampBuf);
            glXGetFBConfigAttrib(dpy, fbConfig[i], GLX_SAMPLES, &samples);

            if ((bestFbIdx < 0) || (sampBuf && (samples > bestSpp))) 
            {
                bestFbIdx = i;
                bestSpp = samples;
            } 

            XFree(xvi);
        }
    }
    GLXFBConfig bestFbConfig = fbConfig[bestFbIdx];
    XFree(fbConfig);

    xvi = glXGetVisualFromFBConfig(dpy, bestFbConfig);
    if (xvi == NULL) 
    {
        std::cerr << "No appropriate visual found" << std::endl;
        XCloseDisplay(dpy);
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    std::cout << "Visual ID with greatest samples per-pixel: " << xvi->visualid << std::endl;
#endif

    /*** Set the XWindow attributes i.e. colormap and event mask ***/

    XSetWindowAttributes xswa;
    xswa.border_pixel = 0;
    xswa.background_pixmap = None;
    xswa.colormap = cmap = XCreateColormap(dpy, RootWindow(dpy, xvi->screen), xvi->visual, AllocNone);
    xswa.event_mask = (
        ExposureMask   | PointerMotionMask | KeyPressMask | 
        KeyReleaseMask | ButtonPressMask   | ButtonReleaseMask
    );

    win = XCreateWindow(
        dpy, RootWindow(dpy, xvi->screen), 
        0, 0, winWidth, winHeight, 0, 
        xvi->depth, 
        InputOutput, 
        xvi->visual, 
        (CWColormap | CWBorderPixel | CWEventMask), 
        &xswa
    );
    if (!win) 
    {
        std::cerr << "Failed to create a window" << std::endl;
        exit(EXIT_FAILURE);
    }

    XFree(xvi);
    XStoreName(dpy, win, winName.c_str());
    XMapWindow(dpy, win);

    return bestFbConfig;
}

void createOpenGLContext(GLXFBConfig &bestFbConfig)
{
    /*
      The OpenGL Architecture Review Board (ARB) has developed certain extension functions (usually 
      platform-specific) which must be retrieved via glXGetProcAddressARB(). Assuming that the argument
      proc_name matches with an existing ARB extension function, a function pointer to that extension
      function is returned.
    */
    const unsigned char* procName = (const unsigned char*)"glXCreateContextAttribsARB";
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 
        (glXCreateContextAttribsARBProc)glXGetProcAddressARB(procName);

    const char *glxExts = glXQueryExtensionsString(dpy, DefaultScreen(dpy));

#ifdef DEBUG
    std::cout << "Late extensions: " << glxExts << std::endl;
#endif

    if (!isGLXExtensionSupported(glxExts, "GLX_ARB_create_context") || !glXCreateContextAttribsARB) 
    {
        std::cerr << "glXCreateContextAttribsARB() not found. Using old GLX context" << std::endl;
        glx = glXCreateNewContext(dpy, bestFbConfig, GLX_RGBA_TYPE, 0, true);
    }
    else 
    {
        int glxAttribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB,   3,
            GLX_CONTEXT_MINOR_VERSION_ARB,   3,
            GLX_CONTEXT_PROFILE_MASK_ARB,    GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            None
        };
        glx = glXCreateContextAttribsARB(dpy, bestFbConfig, 0, true, glxAttribs);
    }

    XSync(dpy, false);

    // Verifying that context is a direct context
    if (!glXIsDirect(dpy, glx)) {
        std::cout << "Indirect GLX rendering context obtained" << std::endl;
    } 
    else 
    {
        std::cout << "Direct GLX rendering context obtained" << std::endl;
    }
    glXMakeCurrent(dpy, win, glx);
}

void processEvents(Camera &camera, bool &getPtrLocation, float playerSpeed)
{
    while (XPending(dpy) > 0)
    {
        XNextEvent(dpy, &xev); // Blocks until event is received

        switch (xev.type)
        {
            case Expose: // Window was being overlapped by another window, but is now exposed
            {
                /* Set affine transform for viewport based on window width/height */
                XGetWindowAttributes(dpy, win, &xwa);
                glViewport(0, 0, xwa.width, xwa.height);
                break;
            }
            case MotionNotify: 
            {
                getPtrLocation = !getPtrLocation;
                if (getPtrLocation) // Need to give cursor a frame of travel time
                {
                    camera.updateRotationFromPointer(dpy, win, xwa);
                }
                break;
            }
            case KeyPress:
            {
                KeySym sym = XLookupKeysym(&xev.xkey, 0);
                switch (sym)
                {
                    case XK_w:
                    {
                        SET_KEY(key_mask, KEY_FORWARD);
                        break;
                    }
                    case XK_s:
                    {
                        SET_KEY(key_mask, KEY_BACKWARD);
                        break;
                    }
                    case XK_a:
                    {
                        SET_KEY(key_mask, KEY_LEFT);
                        break;
                    }
                    case XK_d:
                    {
                        SET_KEY(key_mask, KEY_RIGHT);
                        break;
                    }
                    case XK_space:
                    {
                        SET_KEY(key_mask, KEY_UP);
                        break;
                    }
                    case XK_BackSpace:
                    {
                        SET_KEY(key_mask, KEY_DOWN);
                        break;
                    }
                }
                break;
            }
            case KeyRelease:
            {
                KeySym sym = XLookupKeysym(&xev.xkey, 0);
                switch (sym)
                {
                    case XK_w:
                    {
                        UNSET_KEY(key_mask, KEY_FORWARD);
                        break;
                    }
                    case XK_s:
                    {
                        UNSET_KEY(key_mask, KEY_BACKWARD);
                        break;
                    }
                    case XK_a:
                    {
                        UNSET_KEY(key_mask, KEY_LEFT);
                        break;
                    }
                    case XK_d:
                    {
                        UNSET_KEY(key_mask, KEY_RIGHT);
                        break;
                    }
                    case XK_space:
                    {
                        UNSET_KEY(key_mask, KEY_UP);
                        break;
                    }
                    case XK_BackSpace:
                    {
                        UNSET_KEY(key_mask, KEY_DOWN);
                        break;
                    }
                }
                break;
            }
            case ButtonPress: // Mouse button pressed
            {
                std::cout << "Click detected" << std::endl;
                break;
            }
        }
    }

    if (IS_KEY_SET(key_mask, KEY_FORWARD)) 
    {
        lac_subtract_vec3(&camera.vEye, camera.vEye, camera.vFwdVel);
    }
    if (IS_KEY_SET(key_mask, KEY_BACKWARD)) 
    {
        lac_add_vec3(&camera.vEye, camera.vEye, camera.vFwdVel);
    }
    if (IS_KEY_SET(key_mask, KEY_LEFT)) 
    {
        lac_add_vec3(&camera.vEye, camera.vEye, camera.vRightVel);
    }
    if (IS_KEY_SET(key_mask, KEY_RIGHT)) 
    {
        lac_subtract_vec3(&camera.vEye, camera.vEye, camera.vRightVel);
    }
    if (IS_KEY_SET(key_mask, KEY_UP)) 
    {
        camera.vEye[1] += playerSpeed;
    }
    if (IS_KEY_SET(key_mask, KEY_DOWN)) 
    {
        camera.vEye[1] -= playerSpeed;
    }
}

void renderFrame(Mvp &mvp, glObjects &objs, Camera &camera, size_t indicesSize)
{
    glClearColor(0.2f, 0.4f, 0.4f, 1.0); // Set background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(objs.shader); // Bind shader program for draw call
    glBindVertexArray(objs.vao);

    //cubeRotDeg += (1 % 360);
    //cubeRotRad = lac_deg_to_rad(cubeRotDeg);

    // Model matrix (translate to world space)
    //lac_get_rotation_mat4(&m_cube_rot, cube_rot_rad, (cube_rot_rad * 0.9f), (cube_rot_rad * 0.8f)); 
    //lac_get_translation_mat4(&mCubeTrn, 0.0f, 0.0f, -1.5f); 
    //lac_multiply_mat4(&mvp.mModel, mCubeTrn, mCubeRot);
    lac_get_translation_mat4(&mvp.mModel, 0.0f, 0.0f, -1.5f); 

    int modelLocation = glGetUniformLocation(objs.shader, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_TRUE, mvp.mModel);

    // View matrix (translate to view space)
    camera.calculateViewMatrix();
    int viewLocation = glGetUniformLocation(objs.shader, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_TRUE, mvp.mView->data());

    // Projection matrix (translate to projection space)
    lac_get_projection_mat4(&mvp.mProj, ((float)xwa.height / (float)xwa.width), fov, znear, zfar);

    int projLocation = glGetUniformLocation(objs.shader, "proj");
    glUniformMatrix4fv(projLocation, 1, GL_TRUE, mvp.mProj);

    // Draw call
    glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0);
    glXSwapBuffers(dpy, win);
}
