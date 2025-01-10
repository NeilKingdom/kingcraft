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
 * @brief Checks if a point rests within the bounds of the camera's viewing frustum.
 * @since 01-03-2025
 * @param[in] frustum The camera's viewing frustum
 * @param[in] x The x component of the point being checked
 * @param[in] y The y component of the point being checked
 * @returns True if the point lies within the viewing frustum, otherwise returns false
 */
static bool is_point_inside_frustum(const CullingFrustum &frustum, const float x, const float y)
{
    vec2 a = { frustum.v_eye[0], frustum.v_eye[1] };
    vec2 b = { frustum.v_left[0], frustum.v_left[1] };
    vec2 c = { frustum.v_right[0], frustum.v_right[1] };
    vec2 p = { x, y };

    float areaABC = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]);
    float areaPAB = (a[0] - p[0]) * (b[1] - p[1]) - (a[1] - p[1]) * (b[0] - p[0]);
    float areaPBC = (b[0] - p[0]) * (c[1] - p[1]) - (b[1] - p[1]) * (c[0] - p[0]);
    float areaPCA = (c[0] - p[0]) * (a[1] - p[1]) - (c[1] - p[1]) * (a[0] - p[0]);

    bool has_neg = (areaPAB < 0) || (areaPBC < 0) || (areaPCA < 0);
    bool has_pos = (areaPAB > 0) || (areaPBC > 0) || (areaPCA > 0);

    return !(has_neg && has_pos);
}

/**
 * @brief Sorting function intended for use when sorting the chunk positions list.
 * @since 01-03-2025
 * @param[in] frustum The camera's viewing frustum
 * @param[in] chunk_left Left chunk position
 * @param[in] chunk_right Right chunk position
 * @returns True if the distance between __chunk_left__ and the camera is less than chunk_right,
 *     otherwise returns false
 */
static bool sort_chunk_positions(
    const CullingFrustum &frustum,
    const std::array<float, 2> &chunk_left,
    const std::array<float, 2> &chunk_right
)
{
    float dx_l = chunk_left[0]  - frustum.v_eye[0];
    float dy_l = chunk_left[1]  - frustum.v_eye[1];
    float dx_r = chunk_right[0] - frustum.v_eye[0];
    float dy_r = chunk_right[1] - frustum.v_eye[1];

    return ((dx_l * dx_l) + (dy_l * dy_l)) < ((dx_r * dx_r) + (dy_r * dy_r));
}

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

    Camera camera = Camera();
    Mvp mvp = Mvp(camera);

    GameState &game = GameState::get_instance();
    ChunkFactory &chunk_factory = ChunkFactory::get_instance();

    CullingFrustum frustum;
    auto chunks = std::set<std::shared_ptr<Chunk>>();
    std::vector<std::array<float, 2>> chunk_pos_list;
    std::vector<std::array<float, 2>>::iterator chunk_pos_iter = chunk_pos_list.end();

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

    //auto skybox_tex_paths = cube_map_textures_t{
    //    "res/textures/skybox_right.png",
    //    "res/textures/skybox_left.png",
    //    "res/textures/skybox_front.png",
    //    "res/textures/skybox_back.png",
    //    "res/textures/skybox_top.png",
    //    "res/textures/skybox_bottom.png"
    //};
    auto skybox_tex_paths = cube_map_textures_t{};
    std::fill(skybox_tex_paths.begin(), skybox_tex_paths.end(), "res/textures/test_skybox.png");
    SkyBox skybox = SkyBox(skybox_tex_paths, GL_LINEAR, GL_LINEAR);

    /*** Seed the RNG generator ***/

    srandom(game.seed);

    /*** Game loop ***/

    // Steps:
    // 1. Calculate the view matrix for the current frame based on the mouse cursor deltas
    // 2. Calculate the points which make up the camera's frustum based on an arbitrary render distance 'd'
    // 3. Create a bounding box around the frustum and loop through it. Determine which points lie within the frustum and add them to the list
    // 4. If the chunks list is populated, discard chunks which don't exist in the positions list
    // 5. For any chunks which do not exist in the chunks list, but do exist in the positions list, generate them once per frame
    // 6. Once all chunks have been generated in the positions list, repeat the process

    while (game.is_running)
    {
        auto frame_start = steady_clock::now();
        game.player.speed = KC::PLAYER_BASE_SPEED * (frame_duration / (float)KC::SEC_AS_NANO);

        camera.calculate_view_matrix();

        // Recalculate chunk positions list once all chunks have been rendered
        if (chunk_pos_iter >= chunk_pos_list.end())
        {
            chunk_pos_list.clear();
            frustum = camera.get_frustum_coords(game.render_distance);

            ssize_t min_x = std::min(std::min(frustum.v_eye[0], frustum.v_left[0]), frustum.v_right[0]);
            ssize_t min_y = std::min(std::min(frustum.v_eye[1], frustum.v_left[1]), frustum.v_right[1]);
            ssize_t max_x = std::max(std::max(frustum.v_eye[0], frustum.v_left[0]), frustum.v_right[0]);
            ssize_t max_y = std::max(std::max(frustum.v_eye[1], frustum.v_left[1]), frustum.v_right[1]);

            // Add chunk positions if they belong within the frustum
            for (ssize_t y = min_y; y < max_y; ++y)
            {
                for (ssize_t x = min_x; x < max_x; ++x)
                {
                    if (is_point_inside_frustum(frustum, x, y))
                    {
                        chunk_pos_list.push_back(std::array<float, 2>{ (float)x, (float)y });
                    }
                }
            }

            // Sort chunk positions by distance relative to player
            std::sort(
                chunk_pos_list.begin(),
                chunk_pos_list.end(),
                [&](const std::array<float, 2> &a, const std::array<float, 2> &b) {
                    return sort_chunk_positions(frustum, a, b);
                }
            );

            // Remove chunks which are no longer visible from the chunks list
            // Remove positions which are visible but already exist in the chunks list from the positions list
            for (auto it = chunks.begin(); it != chunks.end();)
            {
                auto find = std::find(
                    chunk_pos_list.begin(),
                    chunk_pos_list.end(),
                    std::array<float, 2>{ it->get()->location[0], it->get()->location[1] }
                );

                if (find == chunk_pos_list.end())
                {
                    it = chunks.erase(it);
                }
                else
                {
                    chunk_pos_list.erase(find);
                    ++it;
                }
            }

            chunk_pos_iter = chunk_pos_list.begin();
        }

        // Determine which faces to render
        chunks.insert(chunk_factory.make_chunk(vec3{ (*chunk_pos_iter)[0], (*chunk_pos_iter)[1], 0.0f }, ALL));
        chunk_pos_iter++;

        process_events(kc_win, camera);
        render_frame(camera, mvp, shaders, chunks, skybox);

        auto frame_end = steady_clock::now();
        frame_duration = duration_cast<nanoseconds>(frame_end - frame_start).count();
        calculate_frame_rate(fps, frames_elapsed, since);

#if 0
        // Switch OpenGL context to ImGui window
        glXMakeCurrent(imgui_win.dpy, imgui_win.win, glx);
        process_imgui_events(imgui_win);
        render_imgui_frame(imgui_win, camera);
        glXMakeCurrent(kc_win.dpy, kc_win.win, glx);
#endif
    }

#ifdef DEBUG
    cleanup(glx, kc_win, imgui_win);
#else
    cleanup(glx, kc_win);
#endif

    return EXIT_SUCCESS;
}
