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
    const std::optional<KCWindow> &imgui_win = std::nullopt
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
    auto chunks = ChunkList<KC::CHUNK_CAP>();

    Camera camera = Camera();
    Mvp mvp = Mvp(camera);

    GameState &game = GameState::get_instance();
    BlockFactory &block_factory = BlockFactory::get_instance();
    ChunkFactory &chunk_factory = ChunkFactory::get_instance();

    int fps;
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

    ShaderProgram block_shader = ShaderProgram("res/shader/block.vs", "res/shader/block.fs");
    ShaderProgram skybox_shader = ShaderProgram("res/shader/skybox.vs", "res/shader/skybox.fs");
    KCShaders shaders = { block_shader, skybox_shader };

    // Create texture atlas

    const std::filesystem::path tex_atlas_path("res/textures/texture_atlas.png");
    Texture texture_atlas = Texture(tex_atlas_path, GL_NEAREST, GL_NEAREST);

    // Create skybox

    auto skybox_tex_paths = cube_map_textures_t{
        "res/textures/skybox_right.png",
        "res/textures/skybox_left.png",
        "res/textures/skybox_front.png",
        "res/textures/skybox_back.png",
        "res/textures/skybox_top.png",
        "res/textures/skybox_bottom.png"
    };
    //auto skybox_tex_paths = cube_map_textures_t();
    std::fill(skybox_tex_paths.begin(), skybox_tex_paths.end(), "res/textures/test_skybox.png");
    SkyBox skybox = SkyBox(skybox_tex_paths, GL_LINEAR, GL_LINEAR);

    // Seed the RNG generator
    srandom(game.seed);
    //output_noise_test();

    CullingFrustum frustum;
    ssize_t chunk_size = game.chunk_size;
    ssize_t x, y;
    ssize_t min_x, min_y, max_x, max_y;
    vec2 look_dir_last = {};
    vec2 chunk_pos_last = {};

    //camera.v_look_dir[0] = -1.0f;
    //camera.v_look_dir[1] =  0.0f;
    //camera.v_look_dir[2] =  0.0f;

    //frustum = camera.get_frustum_coords(4);

    //min_x = std::min(std::min(frustum.v_eye[0], frustum.v_left[0]), frustum.v_right[0]);
    //min_y = std::min(std::min(frustum.v_eye[1], frustum.v_left[1]), frustum.v_right[1]);
    //max_x = std::max(std::max(frustum.v_eye[0], frustum.v_left[0]), frustum.v_right[0]);
    //max_y = std::max(std::max(frustum.v_eye[1], frustum.v_left[1]), frustum.v_right[1]);

    //min_x -= (min_x % chunk_size) + chunk_size;
    //min_y -= (min_y % chunk_size) + chunk_size;
    //max_x = (max_x + chunk_size) - ((max_x + chunk_size) % chunk_size);
    //max_y = (max_y + chunk_size) - ((max_y + chunk_size) % chunk_size);

    //for (y = max_y; y > min_y; y -= chunk_size)
    //{
    //    for (x = max_x; x > min_x; x -= chunk_size)
    //    {
    //        // TODO: Redundant
    //        vec3 v_A = { frustum.v_eye[0], frustum.v_eye[1], 0.0f };
    //        vec3 v_B = { frustum.v_left[0], frustum.v_left[1], 0.0f };
    //        vec3 v_C = { frustum.v_right[0], frustum.v_right[1], 0.0f };

    //        vec3 vA_vC, vB_vA, vC_vB;
    //        vec3 p_vA, p_vB, p_vC;
    //        vec3 v_P = { (float)x, (float)y, 0.0f };
    //        vec3 v_cross = {};

    //        lac_subtract_vec3(vA_vC, v_A, v_C);
    //        lac_subtract_vec3(vB_vA, v_B, v_A);
    //        lac_subtract_vec3(vC_vB, v_C, v_B);

    //        lac_subtract_vec3(p_vA, v_P, v_A);
    //        lac_subtract_vec3(p_vB, v_P, v_B);
    //        lac_subtract_vec3(p_vC, v_P, v_C);

    //        lac_calc_cross_prod(v_cross, vA_vC, p_vC);
    //        if (v_cross[2] < 0.0f)
    //        {
    //            continue;
    //        }

    //        lac_calc_cross_prod(v_cross, vB_vA, p_vA);
    //        if (v_cross[2] < 0.0f)
    //        {
    //            continue;
    //        }

    //        lac_calc_cross_prod(v_cross, vC_vB, p_vB);
    //        if (v_cross[2] < 0.0f)
    //        {
    //            continue;
    //        }

    //        vec3 location = { (float)x / chunk_size, (float)y / chunk_size, 0.0f };
    //        chunks.push_back(chunk_factory.make_chunk(location, ALL));
    //    }
    //}

    /*** Game loop ***/

    while (game.is_running)
    {
        auto frame_start = steady_clock::now();
        game.player.speed = KC::PLAYER_BASE_SPEED * (frame_duration / (float)KC::SEC_AS_NANO);

        camera.calculate_view_matrix();

        if (std::fabsf(look_dir_last[0] - camera.v_look_dir[0]) > 0.25f
            || std::fabsf(look_dir_last[1] - camera.v_look_dir[1]) > 0.25f
            || chunk_pos_last[0] != (camera.v_eye[0] / chunk_size)
            || chunk_pos_last[1] != (camera.v_eye[1] / chunk_size))
        {
            frustum = camera.get_frustum_coords(4);

            min_x = std::min(std::min(frustum.v_eye[0], frustum.v_left[0]), frustum.v_right[0]);
            min_y = std::min(std::min(frustum.v_eye[1], frustum.v_left[1]), frustum.v_right[1]);
            max_x = std::max(std::max(frustum.v_eye[0], frustum.v_left[0]), frustum.v_right[0]);
            max_y = std::max(std::max(frustum.v_eye[1], frustum.v_left[1]), frustum.v_right[1]);

            min_x -= (min_x % chunk_size) + chunk_size;
            min_y -= (min_y % chunk_size) + chunk_size;
            max_x = (max_x + chunk_size) - ((max_x + chunk_size) % chunk_size);
            max_y = (max_y + chunk_size) - ((max_y + chunk_size) % chunk_size);

            x = max_x;
            y = max_y;

            look_dir_last[0] = camera.v_look_dir[0];
            look_dir_last[1] = camera.v_look_dir[1];
            chunk_pos_last[0] = camera.v_eye[0] / chunk_size;
            chunk_pos_last[1] = camera.v_eye[1] / chunk_size;
        }

        bool add_chunk = true;

        // TODO: Redundant
        vec3 v_A = { frustum.v_eye[0], frustum.v_eye[1], 0.0f };
        vec3 v_B = { frustum.v_left[0], frustum.v_left[1], 0.0f };
        vec3 v_C = { frustum.v_right[0], frustum.v_right[1], 0.0f };

        vec3 vA_vC, vB_vA, vC_vB;
        vec3 p_vA, p_vB, p_vC;
        vec3 v_P = { (float)x, (float)y, 0.0f };
        vec3 v_cross = {};

        lac_subtract_vec3(vA_vC, v_A, v_C);
        lac_subtract_vec3(vB_vA, v_B, v_A);
        lac_subtract_vec3(vC_vB, v_C, v_B);

        lac_subtract_vec3(p_vA, v_P, v_A);
        lac_subtract_vec3(p_vB, v_P, v_B);
        lac_subtract_vec3(p_vC, v_P, v_C);

        lac_calc_cross_prod(v_cross, vA_vC, p_vC);
        if (v_cross[2] < 0.0f)
        {
            add_chunk = false;
        }

        lac_calc_cross_prod(v_cross, vB_vA, p_vA);
        if (v_cross[2] < 0.0f)
        {
            add_chunk = false;
        }

        lac_calc_cross_prod(v_cross, vC_vB, p_vB);
        if (v_cross[2] < 0.0f)
        {
            add_chunk = false;
        }

        if (add_chunk)
        {
            vec3 location = { (float)x / chunk_size, (float)y / chunk_size, 0.0f };
            chunks.insert(chunk_factory.make_chunk(location, ALL));
        }

        if (y > min_y)
        {
            if (x > min_x)
            {
                x -= chunk_size;
            }
            else
            {
                x = max_x;
                y -= chunk_size;
            }
        }

        process_events(app_win, camera);
        render_frame(app_win, camera, mvp, game, shaders, chunks, skybox);

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
    cleanup(glx, app_win);
#endif

    return EXIT_SUCCESS;
}
