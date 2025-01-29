/**
 * @file window.cpp
 * @author Neil Kingdom
 * @version 1.0
 * @since 02-03-2024
 * @brief Handles actions related to the game window.
 * Has actions for creating the window, handling events, rendering the frame buffer, etc.
 *
 * References:
 * - https://www.khronos.org/opengl/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX)
 */

#include "window.hpp"

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
    constexpr auto second_as_nano = duration_cast<nanoseconds>(duration<int>(1)).count();
    ++frames_elapsed;

    if (time_elapsed > second_as_nano)
    {
        since = curr_time;
        fps = frames_elapsed;
        frames_elapsed = 0;

        std::cout << "FPS: " << fps << std::endl;
    }
}

/**
 * @brief Creates a new window.
 * @since 02-03-2024
 * @param[in,out] win An instance of KCWindow containing X11-related data
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
        GLX_DOUBLEBUFFER,    false,
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
    XFreeCursor(win.dpy, win.cur.cursor);

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
 * @brief Create an OpenGL context for the given window.
 * @since 02-03-2024
 * @param[in,out] win An instance of KCWindow containing window-related data
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
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 3,
            GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            None
        };
        glx = glx_create_context_attribs_arb(win.dpy, fb_config, 0, true, glx_attribs);
    }

    XSync(win.dpy, false);
    return glx;
}

/**
 * @brief Processes window events in the queue until there aren't any left.
 * @since 02-03-2024
 * @param[in,out] win An instance of KCWindow containing window-related data
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
                    GameState::get_instance().aspect = (float)win.xwa.width / (float)win.xwa.height;
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
                if (key_binds.find(sym) != key_binds.end())
                {
                    SET_BIT(key_mask, key_binds.at(sym));
                }

                if (IS_BIT_SET(key_mask, KEY_EXIT_GAME))
                {
                    GameState::get_instance().is_running = false;
                }
                break;
            }
            case KeyRelease:
            {
                KeySym sym = XLookupKeysym(&win.xev.xkey, 0);
                if (key_binds.find(sym) != key_binds.end())
                {
                    UNSET_BIT(key_mask, key_binds.at(sym));
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

    float magnitude = 0.0f;
    vec3 v_velocity = {};
    vec3 v_right = {};
    vec3 v_fwd = { camera.v_look_dir[0], camera.v_look_dir[1], camera.v_look_dir[2] };

    lac_calc_cross_prod(v_right, KC::v_up, v_fwd);
    lac_normalize_vec3(v_right, v_right);

    if (IS_BIT_SET(key_mask, KEY_MOVE_FORWARD))
    {
        lac_subtract_vec3(v_velocity, v_velocity, v_fwd);
    }
    if (IS_BIT_SET(key_mask, KEY_MOVE_BACKWARD))
    {
        lac_add_vec3(v_velocity, v_velocity, v_fwd);
    }
    if (IS_BIT_SET(key_mask, KEY_MOVE_LEFT))
    {
        lac_subtract_vec3(v_velocity, v_velocity, v_right);
    }
    if (IS_BIT_SET(key_mask, KEY_MOVE_RIGHT))
    {
        lac_add_vec3(v_velocity, v_velocity, v_right);
    }
    if (IS_BIT_SET(key_mask, KEY_MOVE_DOWN))
    {
        lac_subtract_vec3(v_velocity, v_velocity, KC::v_up);
    }
    if (IS_BIT_SET(key_mask, KEY_MOVE_UP))
    {
        lac_add_vec3(v_velocity, v_velocity, KC::v_up);
    }

    lac_calc_magnitude_vec4(&magnitude, v_velocity);
    if (magnitude > 0.0f)
    {
        lac_normalize_vec3(v_velocity, v_velocity);
        lac_multiply_vec3(v_velocity, v_velocity, KC::PLAYER_SPEED_FACTOR);
        lac_add_vec3(camera.v_eye, camera.v_eye, v_velocity);
    }
}

/**
 * @brief Renders the current game frame.
 * @param[in,out] win An instance of KCWindow containing window-related data
 * @param[in,out] camera The currently active camera used for calculating perspective
 * @param[in,out] mvp The Model View Projection matrix
 * @param[in] shader Shader being used for the current draw call
 */
void render_frame(
    Camera &camera,
    Mvp &mvp,
    KCShaders &shaders,
    std::vector<std::shared_ptr<Chunk>> &chunks,
    SkyBox &skybox
)
{
    GameState &game = GameState::get_instance();
    unsigned u_model, u_view, u_proj;

    glClearColor(1.0f, 1.0, 1.0f, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render block data

    shaders.block.bind();

    // Model matrix (translate to world space)
    lac_get_translation_mat4(mvp.m_model, 0.0f, 0.0f, 0.0f - ((float)KC::SEA_LEVEL + (float)KC::PLAYER_HEIGHT));
    u_model = glGetUniformLocation(shaders.block.id, "model");
    glUniformMatrix4fv(u_model, 1, GL_TRUE, mvp.m_model);

    // View matrix (translate to view space)
    u_view = glGetUniformLocation(shaders.block.id, "view");
    glUniformMatrix4fv(u_view, 1, GL_TRUE, mvp.m_view->data());

    // Projection matrix (translate to projection space)
    lac_get_projection_mat4(
        mvp.m_proj,
        game.aspect,
        game.fov,
        game.znear,
        game.zfar
    );
    u_proj = glGetUniformLocation(shaders.block.id, "proj");
    glUniformMatrix4fv(u_proj, 1, GL_TRUE, mvp.m_proj);

    for (auto chunk : chunks)
    {
        glBindVertexArray(chunk->mesh.vao);
        glDrawArrays(GL_TRIANGLES, 0, chunk->mesh.vertices.size());
    }

    shaders.block.unbind();

    // Render skybox

    glDepthFunc(GL_LEQUAL);
    shaders.skybox.bind();

    // Model matrix (translate to world space)
    lac_get_translation_mat4(mvp.m_model, camera.v_eye[0], camera.v_eye[1], camera.v_eye[2]);
    u_model = glGetUniformLocation(shaders.skybox.id, "model");
    glUniformMatrix4fv(u_model, 1, GL_TRUE, mvp.m_model);

    // View matrix (translate to view space)
    u_view = glGetUniformLocation(shaders.skybox.id, "view");
    glUniformMatrix4fv(u_view, 1, GL_TRUE, mvp.m_view->data());

    // Projection matrix (translate to projection space)
    u_proj = glGetUniformLocation(shaders.skybox.id, "proj");
    glUniformMatrix4fv(u_proj, 1, GL_TRUE, mvp.m_proj);

    // Issue draw call
    glBindVertexArray(skybox.mesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, skybox.mesh.vertices.size());

    shaders.skybox.unbind();
    glDepthFunc(GL_LESS);

    // Blit
    glFlush();
}
