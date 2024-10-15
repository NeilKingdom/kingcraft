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

    if (time_elapsed > SEC_AS_NANO)
    {
        since = curr_time;
        fps = frames_elapsed;
        frames_elapsed = 0;

        std::cout << "FPS: " << fps << std::endl;
    }
}

/**
 * @brief Compiles a GLSL shader and returns its id.
 * @since 02-03-2024
 * @param[in] type The type of shader being compiled (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
 * @param[in] source The GLSL source code for the shader
 * @returns The id of the compiled GLSL shader or 0 on failure
 */
unsigned compile_shader(const unsigned type, const std::string source)
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
        char *message = nullptr;

        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        glGetShaderInfoLog(id, length, &length, message);
        std::cerr << "Failed to compile "
            << ((type == GL_VERTEX_SHADER) ? "vertex" : "fragment")
            << " shader: " << std::string(message)
            << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

/**
 * @brief Attaches and links the vertex and fragment shaders to a shader program.
 * @since 02-03-2024
 * @param[in] vertex_shader The vertex shader to be attached
 * @param[in] fragment_shader The fragment shader to be attached
 * @returns The id of the shader program
 */
unsigned create_shader_prog(const std::string vertex_shader, const std::string fragment_shader)
{
    unsigned program = glCreateProgram();
    unsigned vs = compile_shader(GL_VERTEX_SHADER, vertex_shader);
    unsigned fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    checkCompileErrors(program, "PROGRAM");
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void checkCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

/**
 * @brief Checks to see if the X11 extension function is supported by OpenGL.
 * @since 02-03-2024
 * @param ext_list The list of supported extensions to be checked
 * @param ext_name The name of the extension being searched for in __ext_list__
 * @returns True if the extension is supported or false otherwise
 */
bool is_glx_extension_supported(const char *ext_list, const char *ext_name)
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
    XObjects &x_objs,
    const std::string win_name,
    const size_t win_width,
    const size_t win_height
)
{
    // Open the X11 display
    x_objs.dpy = XOpenDisplay(NULL);
    if (x_objs.dpy == NULL)
    {
        std::cerr << "Cannot connect to X server" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Get OpenGL version
    int v_major, v_minor;
    glXQueryVersion(x_objs.dpy, &v_major, &v_minor);

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
    GLXFBConfig *fb_config = glXChooseFBConfig(x_objs.dpy, DefaultScreen(x_objs.dpy), vi_attribs, &fb_count);
    if (fb_config == NULL)
    {
        std::cerr << "Failed to retrieve framebuffer configuration" << std::endl;
        XCloseDisplay(x_objs.dpy);
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
        x_objs.xvi = glXGetVisualFromFBConfig(x_objs.dpy, fb_config[i]);
        if (x_objs.xvi != NULL)
        {
            int samp_buf, samples;
            glXGetFBConfigAttrib(x_objs.dpy, fb_config[i], GLX_SAMPLE_BUFFERS, &samp_buf);
            glXGetFBConfigAttrib(x_objs.dpy, fb_config[i], GLX_SAMPLES, &samples);

            if ((best_fb_idx < 0) || (samp_buf && (samples > best_spp)))
            {
                best_fb_idx = i;
                best_spp = samples;
            }

            XFree(x_objs.xvi);
        }
    }
    GLXFBConfig best_fb_config = fb_config[best_fb_idx];
    XFree(fb_config);

    x_objs.xvi = glXGetVisualFromFBConfig(x_objs.dpy, best_fb_config);
    if (x_objs.xvi == NULL)
    {
        std::cerr << "No appropriate visual found" << std::endl;
        XCloseDisplay(x_objs.dpy);
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    std::cout << "Visual ID with greatest samples per-pixel: " << x_objs.xvi->visualid << std::endl;
#endif

    /*** Create custom invisible cursor ***/

    XColor color = {};
    x_objs.cur.cpmap  = XCreateBitmapFromData(x_objs.dpy, XDefaultRootWindow(x_objs.dpy), "\0", 1, 1);
    x_objs.cur.cursor = XCreatePixmapCursor(
        x_objs.dpy,
        x_objs.cur.cpmap, x_objs.cur.cpmap,
        &color, &color, 0, 0
    );
    XDefineCursor(x_objs.dpy, XDefaultRootWindow(x_objs.dpy), x_objs.cur.cursor);

    /*** Set the XWindow attributes i.e. colormap and event mask ***/

    XSetWindowAttributes xswa;
    xswa.border_pixel = 0;
    xswa.background_pixmap = None;
    xswa.colormap = x_objs.cmap = XCreateColormap(
        x_objs.dpy,
        RootWindow(x_objs.dpy, x_objs.xvi->screen),
        x_objs.xvi->visual,
        AllocNone
    );
    xswa.event_mask = (
        ExposureMask   | PointerMotionMask | KeyPressMask |
        KeyReleaseMask | ButtonPressMask   | ButtonReleaseMask
    );

    x_objs.win = XCreateWindow(
        x_objs.dpy, RootWindow(x_objs.dpy, x_objs.xvi->screen),
        0, 0, win_width, win_height, 0,
        x_objs.xvi->depth,
        InputOutput,
        x_objs.xvi->visual,
        (CWColormap | CWBorderPixel | CWEventMask),
        &xswa
    );
    if (!x_objs.win)
    {
        std::cerr << "Failed to create a window" << std::endl;
        exit(EXIT_FAILURE);
    }

    XFree(x_objs.xvi);
    XStoreName(x_objs.dpy, x_objs.win, win_name.c_str());
    XMapWindow(x_objs.dpy, x_objs.win);

    return best_fb_config;
}

/**
 * @brief Create an OpenGL context for the given X11 window.
 * @since 02-03-2024
 * @param[in,out] x_objs An instance of XObjects containing X-related data
 * @param[in] fb_config The frame buffer configuration that shall be bound to the OpenGL context
 */
void create_opengl_context(XObjects &x_objs, const GLXFBConfig &fb_config)
{
    /*
        The OpenGL Architecture Review Board (ARB) has developed certain extension functions (usually
        platform-specific) which must be retrieved via glXGetProcAddressARB(). Assuming that the argument
        proc_name matches an existing ARB extension function, a function pointer to that extension
        function is returned.
    */
    const unsigned char* proc_name = (const unsigned char*)"glXCreateContextAttribsARB";
    glXCreateContextAttribsARBProc glx_create_context_attribs_arb =
        (glXCreateContextAttribsARBProc)glXGetProcAddressARB(proc_name);

    const char *glx_exts = glXQueryExtensionsString(x_objs.dpy, DefaultScreen(x_objs.dpy));

#ifdef DEBUG
    std::cout << "Late extensions: " << glx_exts << std::endl;
#endif

    if (!is_glx_extension_supported(glx_exts, "GLX_ARB_create_context") || !glx_create_context_attribs_arb)
    {
        std::cerr << "glXCreateContextAttribsARB() not found. Using old GLX context" << std::endl;
        x_objs.glx = glXCreateNewContext(x_objs.dpy, fb_config, GLX_RGBA_TYPE, 0, true);
    }
    else
    {
        int glx_attribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB,   3,
            GLX_CONTEXT_MINOR_VERSION_ARB,   3,
            GLX_CONTEXT_PROFILE_MASK_ARB,    GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            None
        };
        x_objs.glx = glx_create_context_attribs_arb(x_objs.dpy, fb_config, 0, true, glx_attribs);
    }

    XSync(x_objs.dpy, false);
    glXMakeCurrent(x_objs.dpy, x_objs.win, x_objs.glx);
}

/**
 * @brief Processes X events in the queue until there aren't any left.
 * @since 02-03-2024
 * @param[in,out] x_objs An instance of XObjects containing X-related data
 * @param[in,out] camera The active camera which will be updated on MotionNotify events
 */
void process_events(XObjects &x_objs, Camera &camera)
{
    while (XPending(x_objs.dpy) > 0)
    {
        XNextEvent(x_objs.dpy, &x_objs.xev); // Blocks until event is received

        switch (x_objs.xev.type)
        {
            case ConfigureNotify:
            {
                if (x_objs.xev.xconfigure.window == x_objs.win)
                {
                    x_objs.xwa.width = x_objs.xev.xconfigure.width;
                    x_objs.xwa.height = x_objs.xev.xconfigure.height;
                    GameState::aspect = (float)x_objs.xwa.height / (float)x_objs.xwa.width;
                }
                break;
            }
            case Expose: // Window was being overlapped by another window, but is now exposed
            {
                // Set affine transform for viewport based on window width/height
                XGetWindowAttributes(x_objs.dpy, x_objs.win, &x_objs.xwa);
                glViewport(0, 0, x_objs.xwa.width, x_objs.xwa.height);
                break;
            }
            case MotionNotify: // Mouse was moved
            {
                // Only perform this check every other frame
                query_pointer_location = !query_pointer_location;
                if (query_pointer_location)
                {
                    camera.update_rotation_from_pointer(x_objs);
                }
                break;
            }
            case KeyPress:
            {
                KeySym sym = XLookupKeysym(&x_objs.xev.xkey, 0);
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
                KeySym sym = XLookupKeysym(&x_objs.xev.xkey, 0);
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
        lac_multiply_vec3(&v_velocity, v_velocity, GameState::player.speed);
        lac_add_vec3(&camera.v_eye, camera.v_eye, v_velocity);
    }
}

/**
 * @brief Renders the current frame for both OpenGL and optionally ImGui
 * @param[in,out] gl_objs An instance of GLObjects containing OpenGL-related data
 * @param[in,out] x_objs An instance of XObjects containing X-related data
 * @param[in,out] camera The currently active camera used for calculating perspective
 * @param[in,out] mvp The model-view-projection matrix
 * @param[in] indices_size The number of elements within the Element Array Object (EAO)
 */
void render_frame(
    const GLObjects &gl_objs,
    const XObjects &x_objs,
    Camera &camera,
    Mvp &mvp,
    const size_t indices_size,
    const unsigned texture
)
{
    // Set background color
    glClearColor(0.2f, 0.4f, 0.4f, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, texture);

    glUseProgram(gl_objs.shader);   // Bind shader program for draw call
    glBindVertexArray(gl_objs.vao); // Bind vertex array object

    // Model matrix (translate to world space)
    lac_get_translation_mat4(&mvp.m_model, -1.5f, 0.0f, 0.0f);
    int model_uniform = glGetUniformLocation(gl_objs.shader, "model");
    glUniformMatrix4fv(model_uniform, 1, GL_TRUE, mvp.m_model);

    // View matrix (translate to view space)
    camera.calculate_view_matrix();
    int view_uniform = glGetUniformLocation(gl_objs.shader, "view");
    glUniformMatrix4fv(view_uniform, 1, GL_TRUE, mvp.m_view->data());

    // Projection matrix (translate to projection space)
    lac_get_projection_mat4(&mvp.m_proj, GameState::aspect, GameState::fov, znear, zfar);
    int proj_uniform = glGetUniformLocation(gl_objs.shader, "proj");
    glUniformMatrix4fv(proj_uniform, 1, GL_TRUE, mvp.m_proj);

    // Issue draw call
    glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, 0);
    glXSwapBuffers(x_objs.dpy, x_objs.win);
}
