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
        // TODO: Put cursor back to default
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

    mat4 m_trns = {};

    Camera camera = Camera();
    Mvp mvp = Mvp(camera);
    KCShaders shaders = {};
    auto chunks = std::list<Chunk>();

    GameState &game = GameState::get_instance();
    BlockFactory &block_factory = BlockFactory::get_instance();
    ChunkFactory &chunk_factory = ChunkFactory::get_instance();

    int frames_elapsed = 0;
    time_point<steady_clock> since = steady_clock::now();
    nanoseconds::rep frame_duration = 0L;

    // Seed the RNG generator
    srandom(game.seed);
    output_noise_test();

    /*** Window and OpenGL context initialization ***/

    KCWindow app_win;
    KCWindow imgui_win;

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
    std::string vertex_shader;
    std::string fragment_shader;

    // Create block shader

    ifs.open("res/shader/block.vs");
    vertex_shader = std::string(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
    ifs.close();

    ifs.open("res/shader/block.fs");
    fragment_shader = std::string(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
    ifs.close();

    shaders.block = ShaderProgram(vertex_shader, fragment_shader);

    // Create skybox shader

    ifs.open("res/shader/skybox.vs");
    vertex_shader = std::string(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
    ifs.close();

    ifs.open("res/shader/skybox.fs");
    fragment_shader = std::string(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
    ifs.close();

    shaders.skybox = ShaderProgram(vertex_shader, fragment_shader);

    // Create skybox

    auto skybox_tex_paths = std::array<std::filesystem::path, 6> {
        //"res/textures/skybox_right.png",
        //"res/textures/skybox_left.png",
        //"res/textures/skybox_front.png",
        //"res/textures/skybox_back.png",
        //"res/textures/skybox_top.png",
        //"res/textures/skybox_bottom.png"
        "res/textures/test_skybox.png",
        "res/textures/test_skybox.png",
        "res/textures/test_skybox.png",
        "res/textures/test_skybox.png",
        "res/textures/test_skybox.png",
        "res/textures/test_skybox.png"
    };
    SkyBox skybox = SkyBox(skybox_tex_paths, GL_LINEAR, GL_LINEAR);

    // Make chunk

    block_factory.init();

    /*** Game loop ***/

    float x = 1.0f;
    float y = 1.0f;

    while (game.is_running)
    {
        auto frame_start = steady_clock::now();
        game.player.speed = KCConst::PLAYER_BASE_SPEED * (frame_duration / (float)KCConst::SEC_AS_NANO);

        // Create chunks that are nearest to player's position
        if (chunks.size() < 5)
        {
            lac_get_translation_mat4(&m_trns, x, y, 0);
            chunks.push_back(chunk_factory.make_chunk(m_trns, ALL));
            x += 16;
            y += 16;
        }

        process_events(app_win, camera);
        render_frame(app_win, camera, mvp, shaders, chunks, skybox);

        auto frame_end = steady_clock::now();
        frame_duration = duration_cast<nanoseconds>(frame_end - frame_start).count();
        //calculate_frame_rate(fps, frames_elapsed, since);

#if 0
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
