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
 * @returns True if the extension is supported, otherwise returns false
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
