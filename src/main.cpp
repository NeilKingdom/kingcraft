/**
 * @file main.cpp
 * @author Neil Kingdom
 * @version 1.0
 * @since 02-03-2024
 * @brief Entry point of the program.
 * Initializes everything and maintains the gameplay loop.
 */

#include "main.hpp"

/**
 * @brief Cleanup all of the application's resources.
 * @since 02-03-2024
 * @param[in] gl_objs An instance of GLObjects containing OpenGL-related resources
 * @param[in] x_objs An instance of XObjects containing X11-related resources
 * @param[in] im_objs An optional instance of XObjects containing ImGui-related resources
 */
static void cleanup(
    KCWindow &app_win,
    const std::optional<KCWindow> &imgui_win
)
{
    // ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplX11_Shutdown();

    if (imgui_win != std::nullopt)
    {
        // OpenGL context
        ImGui::DestroyContext();

        // X11
        XDestroyWindow(imgui_win->dpy, imgui_win->win);
        XFreeColormap(imgui_win->dpy, imgui_win->cmap);
        XCloseDisplay(imgui_win->dpy);
    }

    // OpenGL context
    glXMakeCurrent(app_win.dpy, None, NULL);
    glXDestroyContext(app_win.dpy, app_win.glx);

    // X11
    XFreeCursor(app_win.dpy, app_win.cur.cursor);
    XFreePixmap(app_win.dpy, app_win.cur.cpmap);

    XDestroyWindow(app_win.dpy, app_win.win);
    XFreeColormap(app_win.dpy, app_win.cmap);
    XCloseDisplay(app_win.dpy);
}

int main()
{
    using namespace std::chrono;

    /*** Variable declarations ***/

    KCWindow app_win;
    KCWindow imgui_win;

    Camera camera = Camera();
    Mvp mvp = Mvp(camera);

    int frames_elapsed = 0;
    time_point<steady_clock> since = steady_clock::now();
    nanoseconds::rep frame_duration = 0L;

    /*** Window and OpenGL context initialization ***/

    GLXFBConfig best_fb_config = create_window(app_win, "KingCraft", 1920, 1080);
    create_opengl_context(app_win, best_fb_config);
#ifdef DEBUG
    (void)create_window(imgui_win, "ImGui", 400, 400);
#endif

    init_imgui(imgui_win);

    // Bind graphics drivers to OpenGL API specification
    // NOTE: Must be placed after a valid OpenGL context has been made current
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Setup callback function for when a OpenGL debug message is received
    if (glDebugMessageCallback)
    {
        glDebugMessageCallback(debug_callback, nullptr);
    }
    else
    {
        std::cerr << "WARNING: glDebugMessageCallback() is unavailable!" << std::endl;
    }

    glEnable(GL_DEBUG_OUTPUT);      // Enable debug output
    //glEnable(GL_CULL_FACE);         // Enable culling
    glEnable(GL_DEPTH_TEST);        // Enable z-ordering via depth buffer

    //glCullFace(GL_BACK);            // Culling algorithm (GL_FRONT = front faces, GL_BACK = back faces)
    //glFrontFace(GL_CCW);            // Front faces (GL_CW = clockwise, GL_CCW = counter clockwise)
    //glDepthFunc(GL_LESS);           // Depth algorithm (GL_LESS = lower zbuffer pixels are rendered on top)

    // Uncomment for wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    /*** Generate textures ***/

#if 0
    // Individual textures
    TextureAtlas atlas(
        KCConst::TEX_SIZE, KCConst::TEX_SIZE,
        "/home/neil/devel/projects/kingcraft/res/textures/texture_atlas.png"
    );

    // Dirt block
    Pixmap_t dirt_block_front  = atlas.get_pixmap_at_id(1);
    Pixmap_t dirt_block_back   = atlas.get_pixmap_at_id(1);
    Pixmap_t dirt_block_right  = atlas.get_pixmap_at_id(1);
    Pixmap_t dirt_block_left   = atlas.get_pixmap_at_id(1);
    Pixmap_t dirt_block_top    = atlas.get_pixmap_at_id(0);
    Pixmap_t dirt_block_bottom = atlas.get_pixmap_at_id(0);

    imc_pixmap_rotate_cw(&dirt_block_right);
    imc_pixmap_rotate_cw(&dirt_block_right);
    imc_pixmap_rotate_cw(&dirt_block_front);
    imc_pixmap_rotate_ccw(&dirt_block_back);

    Pixmap_t dirt_block[6] = {
        dirt_block_back, dirt_block_front, dirt_block_right,
        dirt_block_left, dirt_block_top, dirt_block_bottom
    };

    // Cube map texture
    unsigned dirt_block_texid;
    glGenTextures(1, &dirt_block_texid);
    glBindTexture(GL_TEXTURE_CUBE_MAP, dirt_block_texid);

    // OpenGL   -> liblac
    // (right)  -> (back)
    // (left)   -> (front)
    // (top)    -> (right)
    // (bottom) -> (left)
    // (front)  -> (top)
    // (back)   -> (bottom)
    for (int i = 0; i < KCConst::CUBE_FACES; ++i)
    {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB,
            dirt_block[i].pixmap.width,
            dirt_block[i].pixmap.height,
            0, GL_RGB, GL_UNSIGNED_BYTE,
            dirt_block[i].data
        );
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif

    /*** Create shader program(s) ***/

    auto ifs = std::ifstream();

    ifs.open("res/shader/vertex.shader");
    const std::string vertex_shader(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
    ifs.close();

    ifs.open("res/shader/fragment.shader");
    const std::string fragment_shader(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
    ifs.close();

    ShaderProgram shader = ShaderProgram(vertex_shader, fragment_shader);

    //glUniform1i(glGetUniformLocation(gl_objs.shader, "cubeMap"), 0);

    mat4 m_trns;
    std::memcpy(m_trns, lac_ident_mat4, sizeof(m_trns));
    Block block = BlockFactory::get_instance().make_block(BlockType::DIRT, m_trns, 0xFF);

    /*** Game loop ***/

    while (GameState::get_instance().is_running)
    {
        auto frame_start = steady_clock::now();
        GameState::get_instance().player.speed = KCConst::PLAYER_BASE_SPEED * (frame_duration / (float)KCConst::SEC_AS_NANO);

        process_events(app_win, camera);
        render_frame(block, app_win, camera, mvp);

        auto frame_end = steady_clock::now();
        frame_duration = duration_cast<nanoseconds>(frame_end - frame_start).count();
        //calculate_frame_rate(fps, frames_elapsed, since);

#ifdef DEBUG
        // TODO: Try using imgui_win.glx as the OpenGL context
        // Switch OpenGL context to ImGui window
        glXMakeCurrent(imgui_win.dpy, imgui_win.win, app_win.glx);
        process_imgui_events(imgui_win);
        render_imgui_frame(imgui_win, camera);
        glXMakeCurrent(app_win.dpy, app_win.win, app_win.glx);
#endif
    }

#ifdef DEBUG
    cleanup(app_win, imgui_win);
#else
    cleanup(app_win, std::nullopt);
#endif

    return EXIT_SUCCESS;
}
