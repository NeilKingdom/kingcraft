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
 * @param[in] glx An instance of the OpenGL context object
 * @param[in] app_win An instance of KCWindow containing X11-related resources
 * @param[in] imgui_win An optional instance of KCWindow containing ImGui-related resources
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
    vec2 prev_chunk_location = {};
    auto chunks = std::vector<Chunk>();

    Camera camera = Camera();
    Mvp mvp = Mvp(camera);

    GameState &game = GameState::get_instance();
    BlockFactory &block_factory = BlockFactory::get_instance();
    ChunkFactory &chunk_factory = ChunkFactory::get_instance();

    int frames_elapsed = 0;
    time_point<steady_clock> since = steady_clock::now();
    nanoseconds::rep frame_duration = 0L;

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

    ShaderProgram block_shader = ShaderProgram("res/shader/block.vs", "res/shader/block.fs");
    ShaderProgram skybox_shader = ShaderProgram("res/shader/skybox.vs", "res/shader/skybox.fs");
    KCShaders shaders = { block_shader, skybox_shader };

    // Create texture atlas

    const std::filesystem::path tex_atlas_path("res/textures/texture_atlas.png");
    Texture texture_atlas = Texture(tex_atlas_path, GL_NEAREST, GL_NEAREST);

    // Create skybox

    //auto skybox_tex_paths = cube_map_textures_t{
    //    "res/textures/skybox_right.png",
    //    "res/textures/skybox_left.png",
    //    "res/textures/skybox_front.png",
    //    "res/textures/skybox_back.png",
    //    "res/textures/skybox_top.png",
    //    "res/textures/skybox_bottom.png"
    //};
    auto skybox_tex_paths = cube_map_textures_t();
    std::fill(skybox_tex_paths.begin(), skybox_tex_paths.end(), "res/textures/test_skybox.png");
    SkyBox skybox = SkyBox(skybox_tex_paths, GL_LINEAR, GL_LINEAR);

    // Seed the RNG generator
    srandom(game.seed);
    output_noise_test();

    /*** Game loop ***/

    lac_get_translation_mat4(&m_trns, 16, 16, 0);
    chunks.push_back(chunk_factory.make_chunk(m_trns, ALL));

    while (game.is_running)
    {
        auto frame_start = steady_clock::now();
        game.player.speed = KC::PLAYER_BASE_SPEED * (frame_duration / (float)KC::SEC_AS_NANO);

        process_events(app_win, camera);
        render_frame(app_win, camera, mvp, texture_atlas, shaders, chunks, skybox);

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
