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
    GLXContext glx,
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
        XDestroyWindow(imgui_win.value().dpy, imgui_win.value().win);
        XFreeColormap(imgui_win.value().dpy, imgui_win.value().cmap);
        XCloseDisplay(imgui_win.value().dpy);
    }

    // OpenGL context
    glXMakeCurrent(app_win.dpy, None, NULL);
    glXDestroyContext(app_win.dpy, glx);

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

    GameState &game = GameState::get_instance();
    BlockFactory &block_factory = BlockFactory::get_instance();

    int frames_elapsed = 0;
    time_point<steady_clock> since = steady_clock::now();
    nanoseconds::rep frame_duration = 0L;

    /*** Window and OpenGL context initialization ***/

    GLXFBConfig fb_config = create_window(app_win, "KingCraft", 1920, 1080);
    GLXContext glx = create_opengl_context(app_win, fb_config);
    glXMakeCurrent(app_win.dpy, app_win.win, glx);

#ifdef DEBUG
    (void)create_window(imgui_win, "ImGui", 400, 400);
    init_imgui(imgui_win);
#endif

    // Bind graphics drivers to OpenGL API specification
    // NOTE: Must be placed after a valid OpenGL context has been made current
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << glGetString(GL_VERSION) << std::endl;

    // Enable debug logging
    glEnable(GL_DEBUG_OUTPUT);
    // Set debug logging callback
    if (glDebugMessageCallback)
    {
        glDebugMessageCallback(debug_callback, nullptr);
    }
    else
    {
        std::cerr << "WARNING: glDebugMessageCallback() is unavailable!" << std::endl;
    }

    // Enable depth buffering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable culling
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    // Enable wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    /*** Create shader program(s) ***/

    auto ifs = std::ifstream();

    ifs.open("res/shader/vertex.shader");
    const std::string vertex_shader(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
    ifs.close();

    ifs.open("res/shader/fragment.shader");
    const std::string fragment_shader(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
    ifs.close();

    ShaderProgram shader = ShaderProgram(vertex_shader, fragment_shader);

    PngHndl_t *hndl = nullptr;
    Pixmap_t *pixmap = nullptr;
    std::string file = "/home/neil/devel/projects/kingcraft/res/textures/texture_atlas.png";

    hndl = imc_png_open(file.c_str());
    pixmap = imc_png_parse(hndl);

    imc_pixmap_to_ppm(pixmap, "raster.ppm", Rgb_t{ 255, 255, 255 });
    Texture main_texture = Texture(*pixmap, GL_NEAREST, GL_NEAREST);

    mat4 m_trns = {};
    lac_get_translation_mat4(&m_trns, 0.0f, 0.0f, 0.0f);

    Block block1 = block_factory.make_block(BlockType::DIRT, m_trns, (Face)(FRONT | BACK | LEFT | TOP | BOTTOM));
    block1.mesh.texture = main_texture;

    lac_get_translation_mat4(&m_trns, 0.0f, 1.0f, 0.0f);
    Block block2 = block_factory.make_block(BlockType::DIRT, m_trns, (Face)(FRONT | BACK | RIGHT | TOP | BOTTOM));
    block2.mesh.texture = main_texture;

    std::array<Block, 2> blocks = { block1, block2 };

    /*** Game loop ***/

    while (game.is_running)
    {
        auto frame_start = steady_clock::now();
        game.player.speed = KCConst::PLAYER_BASE_SPEED * (frame_duration / (float)KCConst::SEC_AS_NANO);

        process_events(app_win, camera);
        render_frame(blocks, app_win, camera, mvp, shader);

        auto frame_end = steady_clock::now();
        frame_duration = duration_cast<nanoseconds>(frame_end - frame_start).count();
        //calculate_frame_rate(fps, frames_elapsed, since);

#ifdef DEBUG
        // Switch OpenGL context to ImGui window
        glXMakeCurrent(imgui_win.dpy, imgui_win.win, glx);
        process_imgui_events(imgui_win);
        render_imgui_frame(imgui_win, camera);
        glXMakeCurrent(app_win.dpy, app_win.win, glx);
#endif
    }

#ifdef DEBUG
    cleanup(glx, app_win, imgui_win);
#else
    cleanup(glx, app_win, std::nullopt);
#endif

    return EXIT_SUCCESS;
}
