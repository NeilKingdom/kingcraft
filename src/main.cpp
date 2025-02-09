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
 * @param[in] kc_win An instance of KCWindow containing X11-related resources
 * @param[in] imgui_win An optional instance of KCWindow containing ImGui-related resources
 */
static void cleanup(
    GLXContext glx,
    KCWindow &kc_win,
    const std::optional<KCWindow> &imgui_win = std::nullopt
)
{
    /*** ImGui ***/

    if (imgui_win != std::nullopt)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplX11_Shutdown();
        ImGui::DestroyContext();

        // X11

        XDestroyWindow(imgui_win.value().dpy, imgui_win.value().win);
        XFreeColormap(imgui_win.value().dpy, imgui_win.value().cmap);
        XCloseDisplay(imgui_win.value().dpy);
    }

    /*** OpenGL ***/

    glXMakeCurrent(kc_win.dpy, None, NULL);
    glXDestroyContext(kc_win.dpy, glx);

    // X11

    // Restore normal cursor and free the custom one
    XFreePixmap(kc_win.dpy, kc_win.cur.cpmap);
    XUndefineCursor(kc_win.dpy, XDefaultRootWindow(kc_win.dpy));

    Cursor default_cursor = XCreateFontCursor(kc_win.dpy, XC_arrow);
    XDefineCursor(kc_win.dpy, XDefaultRootWindow(kc_win.dpy), default_cursor);
    XFreeCursor(kc_win.dpy, default_cursor);

    XDestroyWindow(kc_win.dpy, kc_win.win);
    XFreeColormap(kc_win.dpy, kc_win.cmap);
    XCloseDisplay(kc_win.dpy);
}

int main()
{
    using namespace std::chrono;

    /*** Variable declarations ***/

    int fps;
    int frames_elapsed = 0;
    time_point<steady_clock> since = steady_clock::now();
    nanoseconds::rep frame_duration = 0L;
    constexpr auto second_as_nano = duration_cast<nanoseconds>(duration<int>(1)).count();

    Camera camera = Camera();
    CullingFrustum frustum;
    Mvp mvp = Mvp(camera);

    GameState &game = GameState::get_instance();
    ChunkManager chunk_mgr;

    auto &chunks = chunk_mgr.chunks;
    auto &chunk_col_coords = chunk_mgr.chunk_col_coords;
    auto chunk_col_coords_iter = chunk_col_coords.end();

    /*** Window and OpenGL context initialization ***/

    KCWindow kc_win;
    KCWindow imgui_win;

    GLXFBConfig fb_config = create_window(kc_win, "KingCraft", 1920, 1080);
    GLXContext glx = create_opengl_context(kc_win, fb_config);
    glXMakeCurrent(kc_win.dpy, kc_win.win, glx);

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
    //glEnable(GL_DEBUG_OUTPUT);
    glDisable(GL_DEBUG_OUTPUT);
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

    ShaderProgram block_shader  = ShaderProgram("res/shader/block.vs", "res/shader/block.fs");
    ShaderProgram skybox_shader = ShaderProgram("res/shader/skybox.vs", "res/shader/skybox.fs");
    KCShaders shaders = { block_shader, skybox_shader };

    /*** Create texture atlas ***/

    const std::filesystem::path tex_atlas_path("res/textures/texture_atlas.png");
    Texture texture_atlas = Texture(tex_atlas_path, GL_NEAREST, GL_NEAREST);

    /*** Create skybox ***/

    //auto skybox_tex_paths = std::array<std::filesystem::path, 6>{
    //    "res/textures/skybox_right.png",
    //    "res/textures/skybox_left.png",
    //    "res/textures/skybox_front.png",
    //    "res/textures/skybox_back.png",
    //    "res/textures/skybox_top.png",
    //    "res/textures/skybox_bottom.png"
    //};
    auto skybox_tex_paths = std::array<std::filesystem::path, 6>{};
    std::fill(skybox_tex_paths.begin(), skybox_tex_paths.end(), "res/textures/test_skybox.png");
    SkyBox skybox = SkyBox(skybox_tex_paths, GL_LINEAR, GL_LINEAR);

    /*** Other setup ***/

    srandom(game.seed);
    BlockFactory::populate_uv_cache();

    /*** Game loop ***/

    // Steps:
    // 1. Calculate the view matrix for the current frame based on the mouse cursor deltas
    // 2. Calculate the points which make up the camera's frustum based on the render distance
    // 3. Determine which points lie within the frustum and add them to the list
    // 4. If the chunks list is populated, discard chunks which don't exist in the positions list
    // 5. For any chunks which do not exist in the chunks list, but do exist in the positions list,
    //    generate the appropriate chunk column once per frame
    // 6. Once all chunks have been generated in the positions list, repeat the process

    while (game.is_running)
    {
        auto frame_start = steady_clock::now();
        game.player.curr_speed = KC::PLAYER_SPEED_FACTOR * (frame_duration / (float)second_as_nano);

        camera.calculate_view_matrix();

        // Re-calculate chunk positions list once all chunks have been rendered
        if (chunk_col_coords_iter >= chunk_col_coords.end())
        {
            chunk_col_coords.clear();
            frustum = camera.get_frustum_coords(game.render_distance);

            ssize_t min_x = std::min(std::min(frustum.v_eye[0], frustum.v_left[0]), frustum.v_right[0]);
            ssize_t min_y = std::min(std::min(frustum.v_eye[1], frustum.v_left[1]), frustum.v_right[1]);
            ssize_t max_x = std::max(std::max(frustum.v_eye[0], frustum.v_left[0]), frustum.v_right[0]);
            ssize_t max_y = std::max(std::max(frustum.v_eye[1], frustum.v_left[1]), frustum.v_right[1]);

            // Add chunk positions if they belong within the frustum
            chunk_col_coords.reserve((std::fabs(max_x - min_x) * std::fabs(max_y - min_y)));
            for (ssize_t y = min_y; y < max_y; ++y)
            {
                for (ssize_t x = min_x; x < max_x; ++x)
                {
                    if (frustum.is_point_within(vec2{ (float)x, (float)y }))
                    {
                        chunk_col_coords.emplace_back(std::array<float, 2>{ (float)x, (float)y });
                    }
                }
            }
            chunk_col_coords.shrink_to_fit();

            // Sort chunk positions by distance relative to player
            std::sort(
                chunk_col_coords.begin(),
                chunk_col_coords.end(),
                [&](const std::array<float, 2> &a, const std::array<float, 2> &b) {
                    float dx_a = a[0] - frustum.v_eye[0];
                    float dy_a = a[1] - frustum.v_eye[1];
                    float dx_b = b[0] - frustum.v_eye[0];
                    float dy_b = b[1] - frustum.v_eye[1];

                    return ((dx_a * dx_a) + (dy_a * dy_a)) < ((dx_b * dx_b) + (dy_b * dy_b));
                }
            );

            // Remove chunk columns which are no longer visible in the camera's frustum

            chunks.erase(
                std::remove_if(chunks.begin(), chunks.end(), [&](std::shared_ptr<Chunk> &chunk) {
                    auto chunk_pos = std::array<float, 2>{ chunk->location[0], chunk->location[1] };
                    return std::find(chunk_col_coords.begin(), chunk_col_coords.end(), chunk_pos) == chunk_col_coords.end();
                }),
                chunks.end()
            );

            // Remove positions which are visible in the camera's frustum, but who's chunk columns have already been generated

            std::vector<std::array<float, 2>> to_remove;
            for (auto &chunk : chunks)
            {
                auto chunk_pos = std::array<float, 2>{ chunk->location[0], chunk->location[1] };
                to_remove.push_back(chunk_pos);
            }

            chunk_col_coords.erase(
                std::remove_if(chunk_col_coords.begin(), chunk_col_coords.end(), [&](const std::array<float, 2> &pos) {
                    return std::find(to_remove.begin(), to_remove.end(), pos) != to_remove.end();
                }),
                chunk_col_coords.end()
            );

            chunk_col_coords_iter = chunk_col_coords.begin();
        }

        if (!chunk_col_coords.empty())
        {
            auto chunk_col = ChunkFactory::make_chunk_column(vec2{ (*chunk_col_coords_iter)[0], (*chunk_col_coords_iter)[1] });
            chunks.insert(chunks.end(), chunk_col.begin(), chunk_col.end());
        }
        chunk_col_coords_iter++;

        process_events(kc_win, camera);
        render_frame(camera, mvp, shaders, chunks, skybox);

        auto frame_end = steady_clock::now();
        frame_duration = duration_cast<nanoseconds>(frame_end - frame_start).count();
        calculate_frame_rate(fps, frames_elapsed, since);

#if 0
        // Update once per 5 frames
        if (frames_elapsed % 5 == 0)
        {
            glXMakeCurrent(imgui_win.dpy, imgui_win.win, glx);
            process_imgui_events(imgui_win);
            render_imgui_frame(imgui_win, camera);
            glXMakeCurrent(kc_win.dpy, kc_win.win, glx);
        }
#endif
    }

#ifdef DEBUG
    cleanup(glx, kc_win, imgui_win);
#else
    cleanup(glx, kc_win);
#endif

    return EXIT_SUCCESS;
}

