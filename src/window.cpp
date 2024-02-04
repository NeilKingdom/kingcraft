/**
 * Reference: https://www.khronos.org/opengl/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX)
 */

#include "../include/window.hpp"

/**
 * @brief Calculate the framerate and display it once per second (must be called once per frame)
 * @since 02-03-2024
 * @param[in/out] fps The actual FPS count
 * @param[in/out] fElapsed The amount of frames that have elapsed thus far 
 * @param[in/out] timePrev The previous time stamp (used to check if one second has elapsed)
 */
void calculateFrameRate(int &fps, int &fElapsed, std::chrono::steady_clock::time_point &timePrev)
{
    using namespace std::chrono;

    auto timeNow = steady_clock::now();
    auto timeElapsed = duration_cast<nanoseconds>(timeNow - timePrev).count();
    ++fElapsed; 

    if (timeElapsed > SEC_AS_NANO) 
    {
        timePrev = timeNow;
        fps = fElapsed;
        fElapsed = 0;

        std::cout << "FPS: " << fps << std::endl;
    }
}

/**
 * @brief Compiles a GLSL shader and returns its id
 * @since 02-03-2024
 * @param[in] type The type of shader being compiled (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
 * @param[in] source The GLSL source code for the shader as a std::string
 * @returns The shader id
 */
unsigned compileShader(const unsigned type, const std::string source)
{
    assert(type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER);

    int res;
    unsigned id = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);

    glGetShaderiv(id, GL_COMPILE_STATUS, &res); 
    if (res == GL_FALSE) 
    {
        int length;
        std::string message;

        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        glGetShaderInfoLog(id, length, &length, const_cast<char*>(message.c_str()));
        std::cerr << "Failed to compile " 
            << ((type == GL_VERTEX_SHADER) ? "vertex" : "fragment")
            << " shader: " << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

/**
 * @brief Attaches and links the vertex and fragment shaders into a shader program
 * @since 02-03-2024
 * @param[in] vertexShader The vertex shader to be attached (as a std::string)
 * @param[in] fragmentShader The fragment shader to be attached (as a std::string)
 * @returns The program id
 */
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

/**
 * @brief Checks to see if the X11 extension function is supported by OpenGL
 * @since 02-03-2024
 * @param extList The list of supported extensions to be checked
 * @param extName The name of the extension being searched for in extList
 * @returns True if the extension is supported or false otherwise
 */
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

/**
 * @brief Creates a new X11 window
 * @since 02-03-2024
 * @param[in/out] xObjs An instance of xObjects containing X11-related data
 * @param[in] winName The name of the new window
 * @param[in] winWidth The width of the new window
 * @param[in] winHeight The height of the new window 
 * @returns The best available frame buffer configuration for the new window
 */
GLXFBConfig createXWindow(
    xObjects &xObjs, 
    const std::string winName, 
    const size_t winWidth, 
    const size_t winHeight
)
{
    // Open the X11 display
    xObjs.dpy = XOpenDisplay(NULL); 
    if (xObjs.dpy == NULL) 
    {
        std::cerr << "Cannot connect to X server" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Get OpenGL version
    int vMajor, vMinor;
    glXQueryVersion(xObjs.dpy, &vMajor, &vMinor);

    // Specify the visual attributes for the frame buffer configuration
    int viAttribs[] = {
        GLX_X_RENDERABLE,    true,
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
        GLX_DOUBLEBUFFER,    true,
        None
    };

    /*** Takes in our attributes and returns a list of frame buffers loosely matching the criteria ***/
    
    int fbCount;
    GLXFBConfig *fbConfig = glXChooseFBConfig(xObjs.dpy, DefaultScreen(xObjs.dpy), viAttribs, &fbCount);
    if (fbConfig == NULL)
    {
        std::cerr << "Failed to retrieve framebuffer configuration" << std::endl;
        XCloseDisplay(xObjs.dpy);
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
        xObjs.xvi = glXGetVisualFromFBConfig(xObjs.dpy, fbConfig[i]);
        if (xObjs.xvi != NULL) 
        {
            int sampBuf, samples;
            glXGetFBConfigAttrib(xObjs.dpy, fbConfig[i], GLX_SAMPLE_BUFFERS, &sampBuf);
            glXGetFBConfigAttrib(xObjs.dpy, fbConfig[i], GLX_SAMPLES, &samples);

            if ((bestFbIdx < 0) || (sampBuf && (samples > bestSpp))) 
            {
                bestFbIdx = i;
                bestSpp = samples;
            } 

            XFree(xObjs.xvi);
        }
    }
    GLXFBConfig bestFbConfig = fbConfig[bestFbIdx];
    XFree(fbConfig);

    xObjs.xvi = glXGetVisualFromFBConfig(xObjs.dpy, bestFbConfig);
    if (xObjs.xvi == NULL) 
    {
        std::cerr << "No appropriate visual found" << std::endl;
        XCloseDisplay(xObjs.dpy);
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    std::cout << "Visual ID with greatest samples per-pixel: " << xObjs.xvi->visualid << std::endl;
#endif

    /*** Set the XWindow attributes i.e. colormap and event mask ***/

    XSetWindowAttributes xswa;
    xswa.border_pixel = 0;
    xswa.background_pixmap = None;
    xswa.colormap = xObjs.cmap = XCreateColormap(
        xObjs.dpy, 
        RootWindow(xObjs.dpy, xObjs.xvi->screen), 
        xObjs.xvi->visual, 
        AllocNone
    );
    xswa.event_mask = (
        ExposureMask   | PointerMotionMask | KeyPressMask | 
        KeyReleaseMask | ButtonPressMask   | ButtonReleaseMask
    );

    xObjs.win = XCreateWindow(
        xObjs.dpy, RootWindow(xObjs.dpy, xObjs.xvi->screen), 
        0, 0, winWidth, winHeight, 0, 
        xObjs.xvi->depth, 
        InputOutput, 
        xObjs.xvi->visual, 
        (CWColormap | CWBorderPixel | CWEventMask), 
        &xswa
    );
    if (!xObjs.win) 
    {
        std::cerr << "Failed to create a window" << std::endl;
        exit(EXIT_FAILURE);
    }

    XFree(xObjs.xvi);
    XStoreName(xObjs.dpy, xObjs.win, winName.c_str());
    XMapWindow(xObjs.dpy, xObjs.win);

    return bestFbConfig;
}

/**
 * @brief Create an OpenGL context for the given X window
 * @since 02-03-2024
 * @param[in/out] xObjs An instance of xObjects containing X-related data
 * @param[in] fbConfig The frame buffer configuration that shall be bound to the OpenGL context
 */
void createOpenGLContext(xObjects &xObjs, const GLXFBConfig &fbConfig)
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

    const char *glxExts = glXQueryExtensionsString(xObjs.dpy, DefaultScreen(xObjs.dpy));

#ifdef DEBUG
    std::cout << "Late extensions: " << glxExts << std::endl;
#endif

    if (!isGLXExtensionSupported(glxExts, "GLX_ARB_create_context") || !glXCreateContextAttribsARB) 
    {
        std::cerr << "glXCreateContextAttribsARB() not found. Using old GLX context" << std::endl;
        xObjs.glx = glXCreateNewContext(xObjs.dpy, fbConfig, GLX_RGBA_TYPE, 0, true);
    }
    else 
    {
        int glxAttribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB,   3,
            GLX_CONTEXT_MINOR_VERSION_ARB,   3,
            GLX_CONTEXT_PROFILE_MASK_ARB,    GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            None
        };
        xObjs.glx = glXCreateContextAttribsARB(xObjs.dpy, fbConfig, 0, true, glxAttribs);
    }

    XSync(xObjs.dpy, false);

    // Verifying that context is a direct context
    if (!glXIsDirect(xObjs.dpy, xObjs.glx)) {
        std::cout << "Indirect GLX rendering context obtained" << std::endl;
    } 
    else 
    {
        std::cout << "Direct GLX rendering context obtained" << std::endl;
    }

    glXMakeCurrent(xObjs.dpy, xObjs.win, xObjs.glx);
}

/**
 * @brief Processes X events in the queue until there aren't any left
 * @since 02-03-2024
 * @param[in/out] xObjs An instance of xObjects containing X-related data
 * @param[in/out] camera The active camera which will be updated on MotionNotify events
 * @param[in/out] getPtrLocation The pointer location is retrieved every other frame
 * @param[in] playerSpeed The player's current speed
 */
void processEvents(const gameState &state, xObjects &xObjs, Camera &camera, bool &getPtrLocation)
{
    while (XPending(xObjs.dpy) > 0)
    {
        XNextEvent(xObjs.dpy, &xObjs.xev); // Blocks until event is received

        switch (xObjs.xev.type)
        {
            case Expose: // Window was being overlapped by another window, but is now exposed
            {
                /* Set affine transform for viewport based on window width/height */
                XGetWindowAttributes(xObjs.dpy, xObjs.win, &xObjs.xwa);
                glViewport(0, 0, xObjs.xwa.width, xObjs.xwa.height);
                break;
            }
            case MotionNotify: // Mouse was moved
            {
                getPtrLocation = !getPtrLocation;
                if (getPtrLocation) // Need to give cursor a frame of travel time
                {
                    camera.updateRotationFromPointer(xObjs);
                }
                break;
            }
            case KeyPress:
            {
                KeySym sym = XLookupKeysym(&xObjs.xev.xkey, 0);
                switch (sym)
                {
                    case XK_w:
                    {
                        SET_KEY(keyMask, KEY_FORWARD);
                        break;
                    }
                    case XK_s:
                    {
                        SET_KEY(keyMask, KEY_BACKWARD);
                        break;
                    }
                    case XK_a:
                    {
                        SET_KEY(keyMask, KEY_LEFT);
                        break;
                    }
                    case XK_d:
                    {
                        SET_KEY(keyMask, KEY_RIGHT);
                        break;
                    }
                    case XK_space:
                    {
                        SET_KEY(keyMask, KEY_UP);
                        break;
                    }
                    case XK_BackSpace:
                    {
                        SET_KEY(keyMask, KEY_DOWN);
                        break;
                    }
                }
                break;
            }
            case KeyRelease:
            {
                KeySym sym = XLookupKeysym(&xObjs.xev.xkey, 0);
                switch (sym)
                {
                    case XK_w:
                    {
                        UNSET_KEY(keyMask, KEY_FORWARD);
                        break;
                    }
                    case XK_s:
                    {
                        UNSET_KEY(keyMask, KEY_BACKWARD);
                        break;
                    }
                    case XK_a:
                    {
                        UNSET_KEY(keyMask, KEY_LEFT);
                        break;
                    }
                    case XK_d:
                    {
                        UNSET_KEY(keyMask, KEY_RIGHT);
                        break;
                    }
                    case XK_space:
                    {
                        UNSET_KEY(keyMask, KEY_UP);
                        break;
                    }
                    case XK_BackSpace:
                    {
                        UNSET_KEY(keyMask, KEY_DOWN);
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

    if (IS_KEY_SET(keyMask, KEY_FORWARD)) 
    {
        lac_subtract_vec3(&camera.vEye, camera.vEye, camera.vFwdVel);
    }
    if (IS_KEY_SET(keyMask, KEY_BACKWARD)) 
    {
        lac_add_vec3(&camera.vEye, camera.vEye, camera.vFwdVel);
    }
    if (IS_KEY_SET(keyMask, KEY_LEFT)) 
    {
        lac_add_vec3(&camera.vEye, camera.vEye, camera.vRightVel);
    }
    if (IS_KEY_SET(keyMask, KEY_RIGHT)) 
    {
        lac_subtract_vec3(&camera.vEye, camera.vEye, camera.vRightVel);
    }
    if (IS_KEY_SET(keyMask, KEY_UP)) 
    {
        camera.vEye[1] += state.playerSpeed;
    }
    if (IS_KEY_SET(keyMask, KEY_DOWN)) 
    {
        camera.vEye[1] -= state.playerSpeed;
    }
}

/**
 * @brief Renders the current frame for both OpenGL and optionally ImGui
 * @param[in/out] state A struct containing variables associated with the game state
 * @param[in/out] xObjs An instance of xObjects containing X-related data
 * @param[in/out] glObjs An instance of glObjects containing OpenGL-related data
 * @param[in/out] imObjs An optional instance of xObjs containing ImGui-related data
 * @param[in/out] camera The currently active camera used for calculating perspective
 * @param[in/out] mvp The model-view-projection matrix 
 */
void renderFrame(
    const gameState &state,
    const xObjects &xObjs, 
    const std::optional<xObjects> &imObjs, 
    const glObjects &glObjs, 
    Camera &camera, 
    Mvp &mvp,
    const size_t indicesSize
)
{
    glClearColor(0.2f, 0.4f, 0.4f, 1.0); // Set background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (imObjs != std::nullopt)
    {
        // Switch OpenGL context to ImGui window
        glXMakeCurrent(imObjs->dpy, imObjs->win, xObjs.glx);
        renderImGuiFrame(state, *imObjs);
        glXMakeCurrent(xObjs.dpy, xObjs.win, xObjs.glx);
    }

    glUseProgram(glObjs.shader); // Bind shader program for draw call
    glBindVertexArray(glObjs.vao);

    // Model matrix (translate to world space)
    lac_get_translation_mat4(&mvp.mModel, 0.0f, 0.0f, -1.5f); 

    int modelLocation = glGetUniformLocation(glObjs.shader, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_TRUE, mvp.mModel);

    // View matrix (translate to view space)
    camera.calculateViewMatrix();
    int viewLocation = glGetUniformLocation(glObjs.shader, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_TRUE, mvp.mView->data());

    // Projection matrix (translate to projection space)
    float aspect = ((float)xObjs.xwa.height / (float)xObjs.xwa.width);
    lac_get_projection_mat4(&mvp.mProj, aspect, state.fov, state.znear, state.zfar);

    int projLocation = glGetUniformLocation(glObjs.shader, "proj");
    glUniformMatrix4fv(projLocation, 1, GL_TRUE, mvp.mProj);

    // Issue draw call
    glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0);
    glXSwapBuffers(xObjs.dpy, xObjs.win);
}
