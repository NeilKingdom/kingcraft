/**
 * @file game_state.cpp
 * @author Neil Kingdom
 * @since 14-10-2024
 * @version 1.0
 * @brief A singleton class which acts as a context object for the game's state.
 */

#include "game.hpp"

static int count;

static void GLAPIENTRY debug_callback(
   GLenum source,
   GLenum type,
   GLuint id,
   GLenum severity,
   GLsizei length,
   const GLchar *msg,
   const void *args
)
{
   std::cout
       << "GL Callback: "
       << "\ntype = " << (type == GL_DEBUG_TYPE_ERROR ? "GL ERROR" : "GL INFO")
       << "\nseverity = " << severity
       << "\nmessage = " << msg
       << std::endl;
}

/**
 * @brief Returns the single instance of GameState.
 * @since 14-10-2024
 * @returns The GameState instance
 */
Game::Game()
{
    using namespace std::chrono;

    /*** Create windows and create OpenGL context ***/

    KCWindow kc_win;
    KCWindow imgui_win;

    GLXFBConfig fb_config = create_window(kc_win, "KingCraft", 1920, 1080);
    GLXContext glx = create_opengl_context(kc_win, fb_config);
    glXMakeCurrent(kc_win.dpy, kc_win.win, glx);

//#ifdef DEBUG
#if 0
    (void)create_window(imgui_win, "ImGui", 400, 400);
    settings.init_imgui(imgui_win);
#endif

    /*** Bind graphics drivers to OpenGL API specification ***/

    // NOTE: Must be placed after a valid OpenGL context has been made current
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << glGetString(GL_VERSION) << std::endl;

    /*** Variable declarations ***/

    int fps;
    int frames_elapsed = 0;
    time_point<steady_clock> since = steady_clock::now();
    nanoseconds::rep frame_duration = 0L;
    constexpr auto second_as_nano = duration_cast<nanoseconds>(duration<int>(1)).count();

    Camera camera;
    CullingFrustum frustum;
    Mvp mvp = Mvp(camera);

    Settings &settings = Settings::get_instance();
    ChunkManager &chunk_mgr = ChunkManager::get_instance();
    ChunkFactory chunk_factory;
    BlockFactory block_factory;

    auto &chunks = chunk_mgr.chunks;
    auto &chunk_col_coords = chunk_mgr.chunk_col_coords;
    auto chunk_col_coords_iter = chunk_col_coords.end();

    /*** Enable/disable OpenGL options ***/

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

    Shader block_shader  = Shader("res/shader/block.vs", "res/shader/block.fs");
    Shader skybox_shader = Shader("res/shader/skybox.vs", "res/shader/skybox.fs");
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

    srandom(settings.seed);

    /*** Game loop ***/

    // Steps:
    // 1. Calculate the view matrix for the current frame based on the mouse cursor deltas
    // 2. Calculate the points which make up the camera's frustum based on the render distance
    // 3. Determine which points lie within the frustum and add them to the list
    // 4. If the chunks list is populated, discard chunks which don't exist in the positions list
    // 5. For any chunks which do not exist in the chunks list, but do exist in the positions list,
    //    generate the appropriate chunk column once per frame
    // 6. Once all chunks have been generated in the positions list, repeat the process

    while (settings.is_running)
    {
        auto frame_start = steady_clock::now();
        player.curr_speed = KC::PLAYER_SPEED_FACTOR * (frame_duration / (float)second_as_nano);

        camera.calculate_view_matrix();

        // Re-calculate chunk positions list once all chunks have been rendered
        if (chunk_col_coords_iter >= chunk_col_coords.end())
        {
            chunk_col_coords.clear();
            frustum = camera.get_frustum_coords(settings.render_distance);

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

            auto to_remove = std::vector<std::array<float, 2>>{};
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

        // Once per frame, make next chunk column
        if (!chunk_col_coords.empty())
        {
            auto chunk_col = chunk_factory.make_chunk_column(
                block_factory, pn,
                vec2{
                    (*chunk_col_coords_iter)[0],
                    (*chunk_col_coords_iter)[1]
                }
            );
            chunks.insert(chunks.end(), chunk_col.begin(), chunk_col.end());
        }
        else
        {
            plant_trees(chunk_mgr, block_factory);
        }
        chunk_col_coords_iter++;

        process_events(kc_win, camera);
        render_frame(chunk_mgr, camera, mvp, shaders, skybox);

        frame_duration = duration_cast<nanoseconds>(steady_clock::now() - frame_start).count();
        calculate_frame_rate(fps, frames_elapsed, since);

//#ifdef DEBUG
#if 0
        // Update once per 5 frames
        if (frames_elapsed % 5 == 0)
        {
            glXMakeCurrent(imgui_win.dpy, imgui_win.win, glx);
            settings.process_imgui_events(imgui_win);
            settings.render_imgui_frame(imgui_win, camera);
            glXMakeCurrent(kc_win.dpy, kc_win.win, glx);
        }
#endif
    }
}

Game::~Game()
{
    /*** ImGui ***/

#if 0
//#ifdef DEBUG
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplX11_Shutdown();
    ImGui::DestroyContext();

    // X11

    XDestroyWindow(imgui_win.dpy, imgui_win.win);
    XFreeColormap(imgui_win.dpy, imgui_win.cmap);
    XCloseDisplay(imgui_win.dpy);
#endif

    /*** OpenGL ***/

    glXMakeCurrent(kc_win.dpy, None, NULL);
    glXDestroyContext(kc_win.dpy, glx);

    // X11

    // Restore normal cursor and free the custom one
    //XFreePixmap(kc_win.dpy, kc_win.cur.cpmap);
    //XUndefineCursor(kc_win.dpy, XDefaultRootWindow(kc_win.dpy));

    //Cursor default_cursor = XCreateFontCursor(kc_win.dpy, XC_arrow);
    //XDefineCursor(kc_win.dpy, XDefaultRootWindow(kc_win.dpy), default_cursor);
    //XFreeCursor(kc_win.dpy, default_cursor);

    //XDestroyWindow(kc_win.dpy, kc_win.win);
    //XFreeColormap(kc_win.dpy, kc_win.cmap);
    //XCloseDisplay(kc_win.dpy);
}

/**
 * @brief Processes window events in the queue until there aren't any left.
 * @since 02-03-2024
 * @param[in,out] win A reference to an instance of KCWindow containing window-related data
 * @param[in,out] camera The active camera which will be updated on MotionNotify events
 */
void Game::process_events(KCWindow &win, Camera &camera)
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
                    Settings::get_instance().aspect = (float)win.xwa.width / (float)win.xwa.height;
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
                    camera.update_rotation_from_pointer(
                        win,
                        vec2{
                            (float)win.xev.xmotion.x,
                            (float)win.xev.xmotion.y
                        }
                    );
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
                    Settings::get_instance().is_running = false;
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
 * @param[in,out] chunk_mgr A reference to instance of ChunkManager for gathering chunk meshes
 * @param[in,out] camera The currently active camera used for calculating perspective
 * @param[in,out] mvp The Model View Projection matrix
 * @param[in] shaders Contains the available program shaders
 * @param[in] skybox The skybox mesh/entity
 */
void Game::render_frame(
    ChunkManager &chunk_mgr,
    Camera &camera,
    Mvp &mvp,
    KCShaders &shaders,
    SkyBox &skybox
)
{
    Settings &settings = Settings::get_instance();
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
        settings.aspect,
        settings.fov,
        settings.znear,
        settings.zfar
    );
    u_proj = glGetUniformLocation(shaders.block.id, "proj");
    glUniformMatrix4fv(u_proj, 1, GL_TRUE, mvp.m_proj);

    glBindVertexArray(chunk_mgr.terrain_mesh.vao);
    chunk_mgr.update_mesh();
    glDrawArrays(GL_TRIANGLES, 0, chunk_mgr.terrain_mesh.vertices.size());
    glBindVertexArray(0);

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
    glBindVertexArray(0);

    shaders.skybox.unbind();
    glDepthFunc(GL_LESS);

    // Blit
    glFlush();
}

// TODO: Move to private func
static uint32_t fnv1a_hash(const vec3 &chunk_location, const vec3 &block_location) {
    constexpr uint32_t FNV_OFFSET_BASIS = 2166136261u;
    constexpr uint32_t FNV_PRIME = 16777619u;

    unsigned long seed = Settings::get_instance().seed;
    uint32_t hash = FNV_OFFSET_BASIS;

    auto hash_int = [&](int value) {
        // Split int into bytes and hash each
        for (int i = 0; i < 4; ++i) {
            uint8_t byte = (value >> (i * 8)) & 0xFF;
            hash ^= byte;
            hash *= FNV_PRIME;
        }
    };

    vec3 v;
    lac_add_vec3(v, chunk_location, block_location);

    hash_int(v[0]);
    hash_int(v[1]);
    hash_int(v[2]);
    hash_int(seed);

    return hash;
}

void Game::plant_trees(ChunkManager &chunk_mgr, const BlockFactory &block_factory)
{
    Settings &settings = Settings::get_instance();
    ssize_t chunk_size = settings.chunk_size;

    auto tallest_chunks = std::vector<std::shared_ptr<Chunk>>{};
    std::copy_if(
        chunk_mgr.chunks.begin(),
        chunk_mgr.chunks.end(),
        std::back_inserter(tallest_chunks),
        [&](std::shared_ptr<Chunk> &chunk)
        {
            return chunk->is_tallest_in_col;
        }
    );

    for (auto chunk : tallest_chunks)
    {
        for (ssize_t z = 0, _z = (chunk->location[2] * chunk_size); z < chunk_size; ++z, ++_z)
        {
            for (ssize_t y = 0, _y = 1; y < chunk_size; ++y, ++_y)
            {
                for (ssize_t x = 0, _x = 1; x < chunk_size; ++x, ++_x)
                {
                    if (_z == chunk->block_heights[_y][_x])
                    {
                        const unsigned rand_threshold = 578; // The higher, the less probable
                        vec3 tmp_location = { (float)x, (float)y, (float) z };
                        if ((fnv1a_hash(chunk->location, tmp_location) % rand_threshold) == 0)
                        {
                            chunk_mgr.plant_tree(block_factory, chunk, tmp_location);
                        }
                    }
                }
            }
        }

        // TODO: Not accounting for blocks placed outside current chunk
        chunk->update_mesh();
    }
}
