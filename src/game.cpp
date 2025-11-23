/**
 * @file game.cpp
 * @author Neil Kingdom
 * @since 14-10-2024
 * @version 1.0
 * @brief Oversees the execution of the game.
 */

#include "game.hpp"

std::atomic<unsigned> fps = std::atomic<unsigned>(0);
static float delta_time_ms;
static bool query_pointer_location = true;

struct V3Int
{
    int x;
    int y;
    int z;

    bool operator==(const V3Int &v) const
    {
        return x == v.x && y == v.y && z == v.z;
    }
};

namespace std
{
    template <>
    struct hash<V3Int>
    {
        size_t operator()(const V3Int &v) const
        {
            return ((hash<int>()(v.x) ^ (hash<int>()(v.y) << 1)) >> 1) ^ (hash<int>()(v.z) << 1);
        }
    };
}

static std::queue<std::array<float, 3>> chunk_queue;

static bool is_chunk_in_visible_radius(
    const vec2 chunk_location,
    const Camera &camera
) {
    Settings &settings = Settings::get_instance();

    float a = chunk_location[0] - floorf(camera.v_eye[0] / KC::CHUNK_SIZE);
    float b = chunk_location[1] - floorf(camera.v_eye[1] / KC::CHUNK_SIZE);
    float c = sqrtf((a * a) + (b * b));

    return c < settings.render_distance;
}

static void fps_callback()
{
    Settings &settings = Settings::get_instance();
    while (settings.is_running)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "FPS: " << fps.exchange(0) << std::endl;
    }
}

/**
 * @brief OpenGL callback for logging events.
 * @since 14-10-2024
 * @param[in] source Source of the event
 * @param[in] type The type/level of the message
 * @param[in] id Identifier for the message category
 * @param[in] severity The level of severity of the message
 * @param[in] length Size of the message
 * @param[in] msg Message text
 * @param[in] args Additional optional arguments
 */
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
 * @brief Default constructor for Game class.
 * @since 14-10-2024
 */
Game::Game()
{
    /*** Create windows and create OpenGL context ***/

    create_window(kc_win, "KingCraft", 1920, 1080);
    glx = create_opengl_context(kc_win);
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

#ifdef DEBUG
    std::cout << glGetString(GL_VERSION) << std::endl;
#endif

    /*** Variable declarations ***/

    Camera camera;
    Mvp mvp = Mvp(camera);

    Settings &settings = Settings::get_instance();

    /*** Enable/disable OpenGL options ***/

    // Enable debug logging
#if 0
//#ifdef DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    if (glDebugMessageCallback)
    {
        glDebugMessageCallback(debug_callback, nullptr);
    }
    else
    {
        std::cerr << "WARNING: glDebugMessageCallback() is unavailable!" << std::endl;
    }
#endif

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

    block_shader  = Shader("res/shader/block.vs", "res/shader/block.fs");
    skybox_shader = Shader("res/shader/skybox.vs", "res/shader/skybox.fs");

    /*** Create texture atlas ***/

    const std::filesystem::path tex_atlas_path("res/textures/texture_atlas_debug.png");
    Texture texture_atlas = Texture(tex_atlas_path, GL_NEAREST, GL_NEAREST);

    /*** Create skybox ***/

    auto skybox_tex_paths = std::array<std::filesystem::path, KC::CUBE_FACES>{
        "res/textures/skybox_right.png",
        "res/textures/skybox_left.png",
        "res/textures/skybox_front.png",
        "res/textures/skybox_back.png",
        "res/textures/skybox_top.png",
        "res/textures/skybox_bottom.png"
    };
    //auto skybox_tex_paths = std::array<std::filesystem::path, KC::CUBE_FACES>{};
    //std::fill(skybox_tex_paths.begin(), skybox_tex_paths.end(), "res/textures/test_skybox.png");
    SkyBox skybox = SkyBox(skybox_tex_paths, GL_LINEAR, GL_LINEAR);

    /*** Other setup ***/

    srandom(settings.seed);
    fps_thread = std::thread(fps_callback);

    /*** Game loop ***/

    while (settings.is_running)
    {
        auto start = std::chrono::high_resolution_clock::now();

        process_events(camera);
        camera.calculate_view_matrix();
        generate_terrain(camera);
        apply_physics();

        render_frame(camera, mvp, skybox);

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

        auto end = std::chrono::high_resolution_clock::now();
        delta_time_ms = std::chrono::duration<float, std::milli>(end - start).count();
    }

    cleanup();
}

/**
 * @brief Default destructor for Game class.
 * @since 14-10-2024
 */
void Game::cleanup()
{
    fps_thread.join();

    /*** ImGui ***/

#if 0
//#ifdef DEBUG
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplX11_Shutdown();
    ImGui::DestroyContext();

    // X11

    XDestroyWindow(imgui_win.dpy, imgui_win.win);
    XFreeColormap(imgui_win.dpy, imgui_win.xwa.colormap);
    XCloseDisplay(imgui_win.dpy);
#endif

    /*** OpenGL ***/

    glXMakeCurrent(kc_win.dpy, None, NULL);
    glXDestroyContext(kc_win.dpy, glx);

    // X11

    // Restore normal cursor and free the custom one
    XFreePixmap(kc_win.dpy, kc_win.cur.pixmap);
    XUndefineCursor(kc_win.dpy, XDefaultRootWindow(kc_win.dpy));

    Cursor default_cursor = XCreateFontCursor(kc_win.dpy, XC_arrow);
    XDefineCursor(kc_win.dpy, XDefaultRootWindow(kc_win.dpy), default_cursor);
    XFreeCursor(kc_win.dpy, default_cursor);

    XDestroyWindow(kc_win.dpy, kc_win.win);
    XFreeColormap(kc_win.dpy, kc_win.xwa.colormap);
    XCloseDisplay(kc_win.dpy);
}

/**
 * @brief Generates all visible terrain within the camera's viewing frustum.
 *
 *    Steps:
 *    1. Obtain viewing frustum (based on desired render distance)
 *    2. Populate chunk_col_coords list with any (x, y) coordinates that lie within the viewing frustum and
 *       also do not already have previously generated chunks within the global chunks list
 *    3. Sort chunk_col_coords list by distance relative to the player so that chunks that are nearer are
 *       rendered first
 *    4. Remove chunks from global chunk list they are not visible within the frustum for the current iteration
 *    5. Once per frame, make next chunk column and render to the screen. This step continues until all
 *       chunk column positions have been exhausted for the current iteration
 *
 * TODO: params
 */
void Game::generate_terrain(Camera &camera)
{
    Settings &settings = Settings::get_instance();
    ChunkManager &chunk_mgr = ChunkManager::get_instance();
    ChunkFactory &chunk_factory = ChunkFactory::get_instance();

    //auto &chunk_coords_2D = chunk_mgr.chunk_coords_2D;
    auto deferred_list = ChunkSet{};
    //auto loaded_chunk_positions = std::unordered_set<V3Int>{};

    float duration_ms;
    const float tgt_fps_ms = KC::SEC_AS_MS.count() / settings.tgt_fps;

    auto start = std::chrono::high_resolution_clock::now();

    // 1. Obtain visible chunk area
    vec2 top_left = {
        floorf(camera.v_eye[0] / KC::CHUNK_SIZE) - settings.render_distance,
        floorf(camera.v_eye[1] / KC::CHUNK_SIZE) - settings.render_distance
    };

    vec2 btm_right = {
        floorf(camera.v_eye[0] / KC::CHUNK_SIZE) + settings.render_distance,
        floorf(camera.v_eye[1] / KC::CHUNK_SIZE) + settings.render_distance
    };

    // 2. Unload chunks that are no longer visible
    std::erase_if(chunk_mgr.GCL, [&](const std::shared_ptr<Chunk> &chunk)
        {
            return !is_chunk_in_visible_radius(
                vec2{ chunk->location[0], chunk->location[1] },
                camera
            );
        }
    );

    // 3. Gather 3D coordinates for chunks that are still loaded
    //for (const auto &chunk : chunk_mgr.GCL)
    //{
    //    loaded_chunk_positions.insert(V3Int{
    //        (int)(chunk->location[0]),
    //        (int)(chunk->location[1]),
    //        (int)(chunk->location[2]),
    //    });
    //}

    // 4. Optional (but recommended): Sort chunk positions by distance relative to player
    //std::sort(
    //    chunk_coords_2D.begin(),
    //    chunk_coords_2D.end(),
    //    [&](const std::array<float, 2> &a, const std::array<float, 2> &b) {
    //        float dx_a = a[0] - frustum.v_eye[0];
    //        float dy_a = a[1] - frustum.v_eye[1];
    //        float dx_b = b[0] - frustum.v_eye[0];
    //        float dy_b = b[1] - frustum.v_eye[1];

    //        return ((dx_a * dx_a) + (dy_a * dy_a)) < ((dx_b * dx_b) + (dy_b * dy_b));
    //    }
    //);

    // TODO: Gather z coordinate based on biome (min, max) chunk height
    // 6. Queue new chunks that need to be loaded
    for (int z = 8; z <= 10; ++z)
    {
        for (int y = top_left[1]; y < btm_right[1]; ++y)
        {
            for (int x = top_left[0]; x < btm_right[0]; ++x)
            {
                auto chunk_location = std::array<float, 3>{ (float)x, (float)y, (float)z };
                if (!std::any_of(
                    chunk_mgr.GCL.begin(),
                    chunk_mgr.GCL.end(),
                    [&](const std::shared_ptr<Chunk> &chunk)
                    {
                        return V3_EQ(chunk_location, chunk->location);
                    }
                ))
                {
                    chunk_queue.push(chunk_location);
                }
            }
        }
    }

    // 7. Generate as many chunks as possible given target FPS (minimum one chunk)
    do
    {
        if (chunk_queue.empty())
        {
            break;
        }

        auto next = chunk_queue.front();
        chunk_queue.pop();

        if (is_chunk_in_visible_radius(vec2{ next[0], next[1] }, camera) &&
            !std::any_of(
                chunk_mgr.GCL.begin(),
                chunk_mgr.GCL.end(),
                [&](const std::shared_ptr<Chunk> &chunk)
                {
                    return V3_EQ(next, chunk->location);
                }
            )
        )
        {
            vec3 chunk_location = { next[0], next[1], next[2] };
            std::shared_ptr<Chunk> chunk = chunk_factory.make_chunk(chunk_location);
            deferred_list.clear();
            deferred_list.insert(chunk);

            // Don't plant trees if outside biome bounds
            if (chunk->location[2] >= 8 && chunk->location[2] <= 10)
            {
                auto tmp = plant_trees(chunk);
                deferred_list.insert(tmp.begin(), tmp.end());
            }

            for (const auto &_chunk : deferred_list)
            {
                _chunk->update_mesh();
            }
            auto result = chunk_mgr.GCL.insert(chunk);
            if (!result.second)
            {
                std::cerr << "Got here" << std::endl;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        duration_ms = std::chrono::duration<float, std::milli>(end - start).count();
    }
    while (duration_ms < tgt_fps_ms);
}

/**
 * @brief Plants trees within the visible frustum after terrain has been generated.
 *
 * TODO: Params
 */
ChunkSet Game::plant_trees(std::shared_ptr<Chunk> &chunk)
{
    ChunkManager &chunk_mgr = ChunkManager::get_instance();

    auto deferred_list = ChunkSet{};
    const unsigned rand_threshold = 576;

    for (ssize_t y = 0, _y = 1; y < KC::CHUNK_SIZE; ++y, ++_y)
    {
        for (ssize_t x = 0, _x = 1; x < KC::CHUNK_SIZE; ++x, ++_x)
        {
            ssize_t z_offset = chunk->block_heights[_y][_x] / KC::CHUNK_SIZE;
            if (z_offset != chunk->location[2])
            {
                continue;
            }

            ssize_t z = chunk->block_heights[_y][_x] % KC::CHUNK_SIZE;
            vec3 root_location = { (float)x, (float)y, (float)z };
            if (fnv1a_hash(chunk->location, root_location) % rand_threshold == 0)
            {
                auto tmp = chunk_mgr.plant_tree(chunk, root_location);
                deferred_list.insert(tmp.begin(), tmp.end());
            }
        }
    }

    return deferred_list;
}

void Game::apply_physics()
{
    return;
}

/**
 * @brief Processes window events in the queue until there aren't any left.
 * @since 02-03-2024
 * @param[in,out] camera The active camera which will be updated on MotionNotify events
 * TODO: params
 */
void Game::process_events(Camera &camera)
{
    Settings &settings = Settings::get_instance();

    while (XPending(kc_win.dpy) > 0)
    {
        XNextEvent(kc_win.dpy, &kc_win.xev); // Blocks until event is received

        switch (kc_win.xev.type)
        {
            // Window was resized
            case ConfigureNotify:
            {
                if (kc_win.xev.xconfigure.window == kc_win.win)
                {
                    kc_win.xwa.width = kc_win.xev.xconfigure.width;
                    kc_win.xwa.height = kc_win.xev.xconfigure.height;
                    settings.aspect = (float)kc_win.xwa.width / (float)kc_win.xwa.height;
                }
                break;
            }
            // Window was being overlapped by another window, but is now exposed
            case Expose:
            {
                // Set affine transform for viewport based on window width/height
                XGetWindowAttributes(kc_win.dpy, kc_win.win, &kc_win.xwa);
                glViewport(0, 0, kc_win.xwa.width, kc_win.xwa.height);
                break;
            }
            // Mouse was moved
            case MotionNotify:
            {
                // Only perform this check every other frame
                query_pointer_location = !query_pointer_location;
                if (query_pointer_location)
                {
                    camera.update_rotation_from_pointer(kc_win);
                }
                break;
            }
            // Key was pressed
            case KeyPress:
            {
                KeySym sym = XLookupKeysym(&kc_win.xev.xkey, 0);
                if (key_binds.find(sym) != key_binds.end())
                {
                    SET_BIT(key_mask, key_binds.at(sym));
                }

                if (IS_BIT_SET(key_mask, KeyAction::EXIT_GAME))
                {
                    Settings::get_instance().is_running = false;
                }
                break;
            }
            // Key was released
            case KeyRelease:
            {
                KeySym sym = XLookupKeysym(&kc_win.xev.xkey, 0);
                if (key_binds.find(sym) != key_binds.end())
                {
                    UNSET_BIT(key_mask, key_binds.at(sym));
                }
                break;
            }
            // Mouse button was pressed
            case ButtonPress:
            {
                break;
            }
            // Mouse button was released
            case ButtonRelease:
            {
                break;
            }
            default:
            {
                break;
            }
        }
    }

    // Update player movement
    float magnitude = 0.0f;
    vec3 v_velocity = {};
    vec3 v_right = {};
    vec3 v_fwd = { camera.v_look_dir[0], camera.v_look_dir[1], camera.v_look_dir[2] };

    lac_calc_cross_prod(v_right, KC::v_up, v_fwd);
    lac_normalize_vec3(v_right, v_right);

    if (IS_BIT_SET(key_mask, KeyAction::PLYR_FWD))
    {
        lac_subtract_vec3(v_velocity, v_velocity, v_fwd);
    }
    if (IS_BIT_SET(key_mask, KeyAction::PLYR_BACK))
    {
        lac_add_vec3(v_velocity, v_velocity, v_fwd);
    }
    if (IS_BIT_SET(key_mask, KeyAction::PLYR_LEFT))
    {
        lac_subtract_vec3(v_velocity, v_velocity, v_right);
    }
    if (IS_BIT_SET(key_mask, KeyAction::PLYR_RIGHT))
    {
        lac_add_vec3(v_velocity, v_velocity, v_right);
    }
    if (IS_BIT_SET(key_mask, KeyAction::PLYR_DOWN))
    {
        lac_subtract_vec3(v_velocity, v_velocity, KC::v_up);
    }
    if (IS_BIT_SET(key_mask, KeyAction::PLYR_UP))
    {
        lac_add_vec3(v_velocity, v_velocity, KC::v_up);
    }

    lac_calc_magnitude_vec4(&magnitude, v_velocity);
    if (magnitude > 0.0f)
    {
        lac_normalize_vec3(v_velocity, v_velocity);
        lac_multiply_vec3(v_velocity, v_velocity, delta_time_ms * 0.05);
        lac_add_vec3(camera.v_eye, camera.v_eye, v_velocity);
    }
}

/**
 * @brief Renders the current game frame.
 * TODO: params
 */
void Game::render_frame(Camera &camera, Mvp &mvp, SkyBox &skybox)
{
    Settings &settings = Settings::get_instance();
    ChunkManager &chunk_mgr = ChunkManager::get_instance();

    unsigned u_model, u_view, u_proj;

    glClearColor(1.0f, 1.0, 1.0f, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render block data

    block_shader.bind();

    // Model matrix (translate to world space)
    lac_get_translation_mat4(mvp.m_model, 0.0f, 0.0f, 0.0f);
    u_model = glGetUniformLocation(block_shader.id, "model");
    glUniformMatrix4fv(u_model, 1, GL_TRUE, mvp.m_model);

    // View matrix (translate to view space)
    u_view = glGetUniformLocation(block_shader.id, "view");
    glUniformMatrix4fv(u_view, 1, GL_TRUE, mvp.m_view->data());

    // Projection matrix (translate to projection space)
    lac_get_projection_mat4(
        mvp.m_proj,
        settings.aspect,
        settings.fov,
        settings.znear,
        settings.zfar
    );
    u_proj = glGetUniformLocation(block_shader.id, "proj");
    glUniformMatrix4fv(u_proj, 1, GL_TRUE, mvp.m_proj);

    glBindVertexArray(chunk_mgr.terrain_mesh.vao);
    chunk_mgr.update_mesh();
    glDrawArrays(GL_TRIANGLES, 0, chunk_mgr.terrain_mesh.vertices.size());
    glBindVertexArray(0);

    block_shader.unbind();

    // Render skybox

    glDepthFunc(GL_LEQUAL);
    skybox_shader.bind();

    // Model matrix (translate to world space)
    lac_get_translation_mat4(mvp.m_model, camera.v_eye[0], camera.v_eye[1], camera.v_eye[2]);
    u_model = glGetUniformLocation(skybox_shader.id, "model");
    glUniformMatrix4fv(u_model, 1, GL_TRUE, mvp.m_model);

    // View matrix (translate to view space)
    u_view = glGetUniformLocation(skybox_shader.id, "view");
    glUniformMatrix4fv(u_view, 1, GL_TRUE, mvp.m_view->data());

    // Projection matrix (translate to projection space)
    u_proj = glGetUniformLocation(skybox_shader.id, "proj");
    glUniformMatrix4fv(u_proj, 1, GL_TRUE, mvp.m_proj);

    // Issue draw call
    glBindVertexArray(skybox.mesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, skybox.mesh.vertices.size());
    glBindVertexArray(0);

    skybox_shader.unbind();
    glDepthFunc(GL_LESS);

    // Blit
    glFlush();

    // Update FPS thread
    fps++;
}
