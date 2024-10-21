/**
 * @file window.cpp
 * @author Neil Kingdom
 * @version 1.0
 * @since 02-03-2024
 * @brief Handles actions related to the X11 window.
 * Has actions for creating the window, handling events, rendering the frame buffer, etc.
 *
 * References:
 * - https://www.khronos.org/opengl/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX)
 */

#include "window.hpp"

using namespace std::chrono;

/**
 * @brief Prints the current frame rate to stdout.
 * @warning Must be called once per frame.
 * @since 02-03-2024
 * @param[in,out] fps The current frames per second (FPS)
 * @param[in,out] frames_elapsed The amount of frames that have elapsed since __since__
 * @param[in,out] since The amount of time that has passed since the last call to this function
 */
void calculate_frame_rate(int &fps, int &frames_elapsed, steady_clock::time_point &since)
{
    auto curr_time = steady_clock::now();
    auto time_elapsed = duration_cast<nanoseconds>(curr_time - since).count();
    ++frames_elapsed;

    if (time_elapsed > KCConst::SEC_AS_NANO)
    {
        since = curr_time;
        fps = frames_elapsed;
        frames_elapsed = 0;

        std::cout << "FPS: " << fps << std::endl;
    }
}

/**
 * @brief Checks to see if the X11 extension function is supported by OpenGL.
 * @since 02-03-2024
 * @param ext_list The list of supported extensions to be checked
 * @param ext_name The name of the extension being searched for in __ext_list__
 * @returns True if the extension is supported or false otherwise
 */
static bool _is_glx_extension_supported(const char *ext_list, const char *ext_name)
{
    const char *start, *where, *terminator;
    where = strchr(ext_name, ' ');
    start = ext_list;

    if (where || *start == '\0')
    {
        return false;
    }

    while (true)
    {
        where = strstr(start, ext_name);
        if (!where) {
            return false;
        }

        terminator = where + strlen(ext_name);
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
 * @brief Creates a new game window.
 * @since 02-03-2024
 * @param[in,out] x_objs An instance of XObjects containing X11-related data
 * @param[in] win_name The name of the new window
 * @param[in] win_width The width of the new window
 * @param[in] win_height The height of the new window
 * @returns The best available frame buffer configuration for the new window
 */
GLXFBConfig create_window(
    KCWindow &win,
    const std::string win_name,
    const size_t win_width,
    const size_t win_height
)
{
    // Open the X11 display
    win.dpy = XOpenDisplay(NULL);
    if (win.dpy == NULL)
    {
        std::cerr << "Cannot connect to X server" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Get OpenGL version
    int v_major, v_minor;
    glXQueryVersion(win.dpy, &v_major, &v_minor);

    // Specify the visual attributes for the frame buffer configuration
    int vi_attribs[] = {
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

    int fb_count;
    GLXFBConfig *fb_config = glXChooseFBConfig(win.dpy, DefaultScreen(win.dpy), vi_attribs, &fb_count);
    if (fb_config == NULL)
    {
        std::cerr << "Failed to retrieve framebuffer configuration" << std::endl;
        XCloseDisplay(win.dpy);
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    std::cout << "Found " << fb_count << " matching configs" << std::endl;
#endif

    /*** Pick the XVisualInfo struct with the most samples per-pixel from the frame buffer list ***/

    int best_fb_idx = 0;
    int best_spp = 0;

    for (int i = 0; i < fb_count; ++i)
    {
        win.xvi = glXGetVisualFromFBConfig(win.dpy, fb_config[i]);
        if (win.xvi != NULL)
        {
            int samp_buf, samples;
            glXGetFBConfigAttrib(win.dpy, fb_config[i], GLX_SAMPLE_BUFFERS, &samp_buf);
            glXGetFBConfigAttrib(win.dpy, fb_config[i], GLX_SAMPLES, &samples);

            if ((best_fb_idx < 0) || (samp_buf && (samples > best_spp)))
            {
                best_fb_idx = i;
                best_spp = samples;
            }

            XFree(win.xvi);
        }
    }
    GLXFBConfig best_fb_config = fb_config[best_fb_idx];
    XFree(fb_config);

    win.xvi = glXGetVisualFromFBConfig(win.dpy, best_fb_config);
    if (win.xvi == NULL)
    {
        std::cerr << "No appropriate visual found" << std::endl;
        XCloseDisplay(win.dpy);
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    std::cout << "Visual ID with greatest samples per-pixel: " << win.xvi->visualid << std::endl;
#endif

    /*** Create custom invisible cursor ***/

    XColor color = {};
    win.cur.cpmap  = XCreateBitmapFromData(win.dpy, XDefaultRootWindow(win.dpy), "\0", 1, 1);
    win.cur.cursor = XCreatePixmapCursor(
        win.dpy,
        win.cur.cpmap, win.cur.cpmap,
        &color, &color, 0, 0
    );
    XDefineCursor(win.dpy, XDefaultRootWindow(win.dpy), win.cur.cursor);

    /*** Set the XWindow attributes i.e. colormap and event mask ***/

    XSetWindowAttributes xswa;
    xswa.border_pixel = 0;
    xswa.background_pixmap = None;
    xswa.colormap = win.cmap = XCreateColormap(
        win.dpy,
        RootWindow(win.dpy, win.xvi->screen),
        win.xvi->visual,
        AllocNone
    );
    xswa.event_mask = (
        ExposureMask   | PointerMotionMask | KeyPressMask |
        KeyReleaseMask | ButtonPressMask   | ButtonReleaseMask
    );

    win.win = XCreateWindow(
        win.dpy, RootWindow(win.dpy, win.xvi->screen),
        0, 0, win_width, win_height, 0,
        win.xvi->depth,
        InputOutput,
        win.xvi->visual,
        (CWColormap | CWBorderPixel | CWEventMask),
        &xswa
    );
    if (!win.win)
    {
        std::cerr << "Failed to create a window" << std::endl;
        exit(EXIT_FAILURE);
    }

    XFree(win.xvi);
    XStoreName(win.dpy, win.win, win_name.c_str());
    XMapWindow(win.dpy, win.win);

    return best_fb_config;
}

/**
 * @brief Create an OpenGL context for the given X11 window.
 * @since 02-03-2024
 * @param[in,out] win An instance of XObjects containing X-related data
 * @param[in] fb_config The frame buffer configuration that shall be bound to the OpenGL context
 * @returns An OpenGL context for __win__
 */
GLXContext create_opengl_context(KCWindow &win, const GLXFBConfig &fb_config)
{
    GLXContext glx;

    /*
        The OpenGL Architecture Review Board (ARB) has developed certain extension functions (usually
        platform-specific) which must be retrieved via glXGetProcAddressARB(). Assuming that the argument
        proc_name matches an existing ARB extension function, a function pointer to that extension
        function is returned.
    */
    const unsigned char* proc_name = (const unsigned char*)"glXCreateContextAttribsARB";
    glXCreateContextAttribsARBProc glx_create_context_attribs_arb =
        (glXCreateContextAttribsARBProc)glXGetProcAddressARB(proc_name);

    const char *glx_exts = glXQueryExtensionsString(win.dpy, DefaultScreen(win.dpy));

#ifdef DEBUG
    std::cout << "Late extensions: " << glx_exts << std::endl;
#endif

    if (!_is_glx_extension_supported(glx_exts, "GLX_ARB_create_context") || !glx_create_context_attribs_arb)
    {
        std::cerr << "glXCreateContextAttribsARB() not found. Using old GLX context" << std::endl;
        glx = glXCreateNewContext(win.dpy, fb_config, GLX_RGBA_TYPE, 0, true);
    }
    else
    {
        int glx_attribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB,   3,
            GLX_CONTEXT_MINOR_VERSION_ARB,   3,
            GLX_CONTEXT_PROFILE_MASK_ARB,    GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            None
        };
        glx = glx_create_context_attribs_arb(win.dpy, fb_config, 0, true, glx_attribs);
    }

    XSync(win.dpy, false);
    return glx;
}

/**
 * @brief Processes X events in the queue until there aren't any left.
 * @since 02-03-2024
 * @param[in,out] x_objs An instance of XObjects containing X-related data
 * @param[in,out] camera The active camera which will be updated on MotionNotify events
 */
void process_events(KCWindow &win, Camera &camera)
{
    while (XPending(win.dpy) > 0)
    {
        XNextEvent(win.dpy, &win.xev); // Blocks until event is received

        switch (win.xev.type)
        {
            case ConfigureNotify:
            {
                if (win.xev.xconfigure.window == win.win)
                {
                    win.xwa.width = win.xev.xconfigure.width;
                    win.xwa.height = win.xev.xconfigure.height;
                    GameState::get_instance().aspect = (float)win.xwa.height / (float)win.xwa.width;
                }
                break;
            }
            case Expose: // Window was being overlapped by another window, but is now exposed
            {
                // Set affine transform for viewport based on window width/height
                XGetWindowAttributes(win.dpy, win.win, &win.xwa);
                glViewport(0, 0, win.xwa.width, win.xwa.height);
                break;
            }
            case MotionNotify: // Mouse was moved
            {
                // Only perform this check every other frame
                query_pointer_location = !query_pointer_location;
                if (query_pointer_location)
                {
                    camera.update_rotation_from_pointer(win);
                }
                break;
            }
            case KeyPress:
            {
                KeySym sym = XLookupKeysym(&win.xev.xkey, 0);
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
                KeySym sym = XLookupKeysym(&win.xev.xkey, 0);
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

    vec3 v_fwd      = { 0 };
    vec3 v_right    = { 0 };
    vec3 v_up       = { 0 };
    vec3 v_velocity = { 0 };
    float magnitude = 0.0f;

    std::memcpy(v_fwd, camera.v_look_dir, sizeof(v_fwd));
    lac_normalize_vec3(&v_fwd, v_fwd);

    lac_calc_cross_prod(&v_right, camera.v_up, v_fwd);
    lac_normalize_vec3(&v_right, v_right);

    lac_calc_cross_prod(&v_up, v_right, v_fwd);
    lac_normalize_vec3(&v_up, v_up);

    if (IS_KEY_SET(key_mask, KEY_FORWARD))
    {
        lac_subtract_vec3(&v_velocity, v_velocity, v_fwd);
    }
    else if (IS_KEY_SET(key_mask, KEY_BACKWARD))
    {
        lac_add_vec3(&v_velocity, v_velocity, v_fwd);
    }
    if (IS_KEY_SET(key_mask, KEY_LEFT))
    {
        lac_subtract_vec3(&v_velocity, v_velocity, v_right);
    }
    else if (IS_KEY_SET(key_mask, KEY_RIGHT))
    {
        lac_add_vec3(&v_velocity, v_velocity, v_right);
    }
    if (IS_KEY_SET(key_mask, KEY_DOWN))
    {
        lac_subtract_vec3(&v_velocity, v_velocity, v_up);
    }
    else if (IS_KEY_SET(key_mask, KEY_UP))
    {
        lac_add_vec3(&v_velocity, v_velocity, v_up);
    }

    lac_calc_magnitude_vec4(&magnitude, v_velocity);
    if (magnitude > 0.0f)
    {
        lac_normalize_vec3(&v_velocity, v_velocity);
        lac_multiply_vec3(&v_velocity, v_velocity, KCConst::PLAYER_BASE_SPEED);
        lac_add_vec3(&camera.v_eye, camera.v_eye, v_velocity);
    }
}

/**
 * @brief Renders the current frame for both OpenGL and optionally ImGui
 * @param[in,out] win An instance of XObjects containing X-related data
 * @param[in,out] camera The currently active camera used for calculating perspective
 * @param[in,out] mvp The model-view-projection matrix
 */
void render_frame(
    const Block &block,
    const KCWindow &win,
    Camera &camera,
    Mvp &mvp
)
{
    // Set background color
    glClearColor(0.2f, 0.4f, 0.4f, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind the shader program and any VAOs and textures
    block.mesh.shader.bind();
    block.mesh.texture.bind();
    glBindVertexArray(block.mesh.vao);

    // Model matrix (translate to world space)
    lac_get_translation_mat4(&mvp.m_model, -1.5f, 0.0f, 0.0f);
    int model_uniform = glGetUniformLocation(block.mesh.shader.id, "model");
    glUniformMatrix4fv(model_uniform, 1, GL_TRUE, mvp.m_model);

    // View matrix (translate to view space)
    camera.calculate_view_matrix();
    int view_uniform = glGetUniformLocation(block.mesh.shader.id, "view");
    glUniformMatrix4fv(view_uniform, 1, GL_TRUE, mvp.m_view->data());

    // Projection matrix (translate to projection space)
    lac_get_projection_mat4(
        &mvp.m_proj,
        GameState::get_instance().aspect,
        GameState::get_instance().fov,
        GameState::get_instance().znear,
        GameState::get_instance().zfar
    );
    int proj_uniform = glGetUniformLocation(block.mesh.shader.id, "proj");
    glUniformMatrix4fv(proj_uniform, 1, GL_TRUE, mvp.m_proj);

    // Issue draw call
    glDrawArrays(GL_TRIANGLES, 0, block.mesh.vertices);
    glXSwapBuffers(win.dpy, win.win);

    // Unbind the shader program and any VAOs and textures
    block.mesh.shader.unbind();
    block.mesh.texture.unbind();
    glBindVertexArray(0);
}
