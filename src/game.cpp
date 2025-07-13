/**
 * @file game.cpp
 * @author Neil Kingdom
 * @since 14-10-2024
 * @version 1.0
 * @brief Oversees the execution of the game.
 */

#include "game.hpp"

std::atomic<unsigned> fps = std::atomic<unsigned>(0);

static auto sec_as_millis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(1));
static std::queue<std::array<float, 3>> chunk_queue;
float avg_chunk_proc_time = 0.0f;

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

// TODO: Move to private func
static uint32_t fnv1a_hash(const vec3 chunk_location, const vec3 block_location)
{
    constexpr uint32_t FNV_OFFSET_BASIS = 2166136261u;
    constexpr uint32_t FNV_PRIME = 16777619u;
    uint32_t hash = FNV_OFFSET_BASIS;

    auto hash_int = [&](int value) {
        // Split int into bytes and hash each
        for (int i = 0; i < 4; ++i) {
            uint8_t byte = (value >> (i * 8)) & 0xFF;
            hash ^= byte;
            hash *= FNV_PRIME;
        }
    };

    vec3 v{};
    lac_add_vec3(v, chunk_location, block_location);

    hash_int(v[0]);
    hash_int(v[1]);
    hash_int(v[2]);

    return hash;
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

    GLXFBConfig fb_config = create_window(kc_win, "KingCraft", 1920, 1080);
    glx = create_opengl_context(kc_win, fb_config);
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

    Camera camera;
    Frustum2D frustum;
    Mvp mvp = Mvp(camera);

    BlockFactory block_factory;
    ChunkFactory chunk_factory;
    Settings &settings = Settings::get_instance();
    ChunkManager &chunk_mgr = ChunkManager::get_instance();

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
    fps_thread = std::thread(KC::fps_callback);

    /*** Game loop ***/

    while (settings.is_running)
    {
        process_events(camera, settings);
        camera.calculate_view_matrix();
        frustum = camera.get_frustum2D(settings.render_distance);
        generate_terrain(chunk_mgr, chunk_factory, block_factory, frustum, settings);
        //plant_trees(chunk_mgr, block_factory, camera, mvp, shaders, skybox, settings);
        apply_physics();
        render_frame(chunk_mgr, camera, mvp, shaders, skybox, settings);

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
    XFreeColormap(imgui_win.dpy, imgui_win.cmap);
    XCloseDisplay(imgui_win.dpy);
#endif

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
void Game::generate_terrain(
    ChunkManager &chunk_mgr,
    ChunkFactory &chunk_factory,
    BlockFactory &block_factory,
    Frustum2D &frustum,
    Settings &settings
)
{
    auto &chunks = chunk_mgr.GCL;
    auto &chunk_coords_2D = chunk_mgr.chunk_coords_2D;
    auto chunk_coords_3D = std::unordered_set<V3Int>{};

    float time_remaining;
    auto start = std::chrono::high_resolution_clock::now();

    // 1. Obtain viewing frustum bounds
    ssize_t min_x = std::min(std::min(frustum.v_eye[0], frustum.v_left[0]), frustum.v_right[0]);
    ssize_t min_y = std::min(std::min(frustum.v_eye[1], frustum.v_left[1]), frustum.v_right[1]);
    ssize_t max_x = std::max(std::max(frustum.v_eye[0], frustum.v_left[0]), frustum.v_right[0]);
    ssize_t max_y = std::max(std::max(frustum.v_eye[1], frustum.v_left[1]), frustum.v_right[1]);

    // 2. Cull chunks that are no longer visible
    std::erase_if(chunks, [&](const std::shared_ptr<Chunk> &chunk)
        {
            return !frustum.is_point_within(vec2{ chunk->location[0], chunk->location[1] });
        }
    );

    // 3. Gather 3D coordinates for chunks that are visible and exist already
    for (const auto &chunk : chunks)
    {
        chunk_coords_3D.insert(V3Int{
            (int)(chunk->location[0]),
            (int)(chunk->location[1]),
            (int)(chunk->location[2]),
        });
    }

    // 4. Determine visible chunk columns
    chunk_coords_2D.clear();
    chunk_coords_2D.reserve((std::fabs(max_x - min_x) * std::fabs(max_y - min_y)));
    for (ssize_t y = min_y; y < max_y; ++y)
    {
        for (ssize_t x = min_x; x < max_x; ++x)
        {
            if (frustum.is_point_within(vec2{ (float)x, (float)y }))
            {
                chunk_coords_2D.emplace_back(std::array<float, 2>{ (float)x, (float)y });
            }
        }
    }

    // 5. Optional: Sort chunk positions by distance relative to player
    std::sort(
        chunk_coords_2D.begin(),
        chunk_coords_2D.end(),
        [&](const std::array<float, 2> &a, const std::array<float, 2> &b) {
            float dx_a = a[0] - frustum.v_eye[0];
            float dy_a = a[1] - frustum.v_eye[1];
            float dx_b = b[0] - frustum.v_eye[0];
            float dy_b = b[1] - frustum.v_eye[1];

            return ((dx_a * dx_a) + (dy_a * dy_a)) < ((dx_b * dx_b) + (dy_b * dy_b));
        }
    );

    // TODO: Gather z coordinate based on biome (min, max) chunk height
    // 6. Queue 3D chunk positions
    for (const auto &coord : chunk_coords_2D)
    {
        for (int z = 8; z <= 10; ++z)
        {
            V3Int key{ (int)coord[0], (int)coord[1], z };
            if (!chunk_coords_3D.contains(key))
            {
                chunk_queue.push({ coord[0], coord[1], (float)z });
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

        V3Int key{ (int)next[0], (int)next[1], (int)next[2] };
        if (!chunk_coords_3D.contains(key)
            && frustum.is_point_within(vec2{ next[0], next[1] }))
        {
            chunks.insert(chunk_factory.make_chunk(block_factory, pn, vec3{ next[0], next[1], next[2] }));
            chunk_coords_3D.insert(key);
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> duration = end - start;
        time_remaining = (sec_as_millis.count() / settings.tgt_fps) - duration.count();
    }
    while (time_remaining >= avg_chunk_proc_time);
}

/**
 * @brief Plants trees within the visible frustum after terrain has been generated.
 *
 * TODO: Params
 */
void Game::plant_trees(
    ChunkManager &chunk_mgr,
    BlockFactory &block_factory,
    Camera &camera,
    Mvp &mvp,
    KCShaders &shaders,
    SkyBox &skybox,
    Settings &settings
)
{
    ChunkSet defered_list{};
    ssize_t chunk_size = settings.chunk_size;
    auto &chunk_coords_2D = chunk_mgr.chunk_coords_2D;

    for (auto it = chunk_coords_2D.begin(); it != chunk_coords_2D.end(); ++it)
    {
        defered_list.clear();

        for (ssize_t y = 0; y < chunk_size; ++y)
        {
            for (ssize_t x = 0; x < chunk_size; ++x)
            {
                float z = pn.octave_perlin(
                    -(it->at(0) * chunk_size) + x,
                     (it->at(1) * chunk_size) + y,
                     0.8f, 0.05f, 3,
                     KC::SEA_LEVEL, KC::SEA_LEVEL + (chunk_size * 3)
                );

                vec3 root_location = { (float)x, (float)y, (float)((ssize_t)z % chunk_size) };
                vec3 chunk_location = { (float)it->at(0), (float)it->at(1), (float)((ssize_t)(z / chunk_size)) };

                const unsigned rand_threshold = 576; // The higher, the less probable
                if (fnv1a_hash(chunk_location, root_location) % rand_threshold == 0)
                {
                    auto lookup = std::make_shared<Chunk>(chunk_location);
                    auto needle = chunk_mgr.GCL.find(lookup);
                    if (needle != chunk_mgr.GCL.end())
                    {
                        std::shared_ptr<Chunk> tmp_chunk = *needle;
                        auto defered = chunk_mgr.plant_tree(tmp_chunk, block_factory, pn, root_location);
                        defered_list.insert(defered.begin(), defered.end());
                    }
                }
            }
        }

        for (auto &chunk : defered_list)
        {
            chunk->update_mesh();
        }

        apply_physics();
        process_events(camera, settings);
        camera.calculate_view_matrix();
        render_frame(chunk_mgr, camera, mvp, shaders, skybox, settings);
    }
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
void Game::process_events(Camera &camera, Settings &settings)
{
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
                    camera.update_rotation_from_pointer(
                        kc_win,
                        vec2{
                            (float)kc_win.xev.xmotion.x,
                            (float)kc_win.xev.xmotion.y
                        }
                    );
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

                if (IS_BIT_SET(key_mask, KEY_EXIT_GAME))
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
        //lac_multiply_vec3(v_velocity, v_velocity, delta_time * 0.05);
        lac_multiply_vec3(v_velocity, v_velocity, KC::PLAYER_SPEED_FACTOR * 0.5);
        lac_add_vec3(camera.v_eye, camera.v_eye, v_velocity);
    }
}

/**
 * @brief Renders the current game frame.
 * TODO: params
 */
void Game::render_frame(
    ChunkManager &chunk_mgr,
    Camera &camera,
    Mvp &mvp,
    KCShaders &shaders,
    SkyBox &skybox,
    Settings &settings
)
{
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

    // Update FPS thread
    fps++;
}
