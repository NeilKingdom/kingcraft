/**
 * @file game.cpp
 * @author Neil Kingdom
 * @since 14-10-2024
 * @version 1.0
 * @brief Oversees the execution of the game.
 */

#include "game.hpp"

static bool query_pointer_location = true;
static std::atomic<unsigned> fps = std::atomic<unsigned>(0);
static float delta_time_ms;

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
    // Unused
    (void)source;
    (void)id;
    (void)length;
    (void)args;

    std::cout
        << "debug_callback(): "
        << "\ntype = "     << ((type == GL_DEBUG_TYPE_ERROR) ? "GL ERROR" : "GL INFO")
        << "\nseverity = " << severity
        << "\nmessage = "  << msg
        << std::endl;
}

/**
 * @brief Default constructor for Game class.
 * @since 14-10-2024
 */
Game::Game()
{
    Settings &settings = Settings::get_instance();

    /*** Create windows and create OpenGL context ***/

    this->kc_win = create_window("KingCraft", 1920, 1080);
    this->glx = create_opengl_context(this->kc_win);
    glXMakeCurrent(this->kc_win.dpy, this->kc_win.win, this->glx);

//#ifdef DEBUG
#if 0
    this->imgui_win = create_window("ImGui", 400, 400);
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

    this->block_shader  = Shader("res/shader/block.vs", "res/shader/block.fs");
    this->skybox_shader = Shader("res/shader/skybox.vs", "res/shader/skybox.fs");

    /*** Create texture atlas ***/

    const std::filesystem::path tex_atlas_path("res/textures/texture_atlas.png");
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
    this->fps_thread = std::thread(fps_callback);

    /*** Game loop ***/

    Camera camera;
    Mvp mvp = Mvp(camera);
    ChunkManager &chunk_mgr = ChunkManager::get_instance();
    std::queue<ChunkMapKey> chunk_queue;

    while (settings.is_running)
    {
        auto start = std::chrono::high_resolution_clock::now();

        process_events(camera);
        camera.calculate_view_matrix();
        generate_terrain(camera, chunk_queue);
        apply_physics(camera);
        chunk_mgr.bind_terrain_mesh();
        render_frame(camera, mvp, skybox);

#if 0
//#ifdef DEBUG
        // Update ImGui window once per 5 frames
        if (fps % 5 == 0)
        {
            glXMakeCurrent(this->imgui_win.dpy, this->imgui_win.win, this->glx);
            settings.process_imgui_events(this->imgui_win);
            settings.render_imgui_frame(this->imgui_win, camera);
            glXMakeCurrent(this->kc_win.dpy, this->kc_win.win, this->glx);
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
    this->fps_thread.join();

    /*** ImGui window ***/

#if 0
//#ifdef DEBUG
    // Destroy context
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplX11_Shutdown();
    ImGui::DestroyContext();

    // Destroy window, colormap, and display
    XDestroyWindow(this->imgui_win.dpy, this->imgui_win.win);
    XFreeColormap(this->imgui_win.dpy, this->imgui_win.xwa.colormap);
    XCloseDisplay(this->imgui_win.dpy);
#endif

    /*** KingCraft window ***/

    // Destroy context
    glXMakeCurrent(this->kc_win.dpy, None, NULL);
    glXDestroyContext(this->kc_win.dpy, this->glx);

    // Restore normal cursor and free the custom one
    XFreePixmap(this->kc_win.dpy, this->kc_win.cur.pixmap);
    XUndefineCursor(this->kc_win.dpy, XDefaultRootWindow(this->kc_win.dpy));
    Cursor default_cursor = XCreateFontCursor(this->kc_win.dpy, XC_arrow);
    XDefineCursor(this->kc_win.dpy, XDefaultRootWindow(this->kc_win.dpy), default_cursor);
    XFreeCursor(this->kc_win.dpy, default_cursor);

    // Destroy window, colormap, and display
    XDestroyWindow(this->kc_win.dpy, this->kc_win.win);
    XFreeColormap(this->kc_win.dpy, this->kc_win.xwa.colormap);
    XCloseDisplay(this->kc_win.dpy);
}

/**
 * @brief Generates terrain according to the specified render distance.
 * TODO: params
 */
void Game::generate_terrain(Camera &camera, std::queue<ChunkMapKey> &chunk_queue)
{
    Settings &settings = Settings::get_instance();
    ChunkManager &chunk_mgr = ChunkManager::get_instance();
    ChunkFactory &chunk_factory = ChunkFactory::get_instance();

    float duration_ms;
    const float tgt_fps_ms = (float)KC::SEC_AS_MS.count() / settings.tgt_fps;

    auto start = std::chrono::high_resolution_clock::now();

    // 1. Obtain visible chunk area
    Vec2_t top_left = { .v = {
        floorf(camera.v_eye.x / KC::CHUNK_SIZE) - settings.render_distance,
        floorf(camera.v_eye.y / KC::CHUNK_SIZE) - settings.render_distance
    }};
    Vec2_t btm_right = { .v = {
        floorf(camera.v_eye.x / KC::CHUNK_SIZE) + settings.render_distance,
        floorf(camera.v_eye.y / KC::CHUNK_SIZE) + settings.render_distance
    }};

    // 2. Unload chunks that are no longer visible
    std::erase_if(chunk_mgr.GCL.map, [&](const auto &kv_pair)
    {
        const auto &chunk = kv_pair.second;

        // Can't unload if chunk contains folliage for another chunk that hasn't been unloaded yet
        if (!chunk->tree_ref.expired())
        {
            return false;
        }

        return !camera.is_chunk_in_visible_radius(chunk->location);
    });

    // TODO:
    // 3. Optional (but recommended): Sort chunk positions by distance relative to player

    // TODO: Gather z coordinate based on biome (min, max) chunk height
    // 4. Queue new chunks that need to be loaded
    for (int z = 8; z <= 10; ++z)
    {
        for (int y = top_left.y; y < btm_right.y; ++y)
        {
            for (int x = top_left.x; x < btm_right.x; ++x)
            {
                chunk_queue.push(ChunkMapKey((Vec3_t){ .v = { (float)x, (float)y, (float)z }}));
            }
        }
    }

    // 5. Generate as many chunks as possible given target FPS (minimum one chunk)
    do
    {
        if (chunk_queue.empty())
        {
            break;
        }

        auto next = chunk_queue.front();
        chunk_queue.pop();
        auto chunk_location = Vec3_t{ .v = { (float)next.x, (float)next.y, (float)next.z }};

        // Only process chunks that are still within visible radius and don't already exist in the GCL
        if (camera.is_chunk_in_visible_radius(chunk_location) && !chunk_mgr.GCL.contains(chunk_location))
        {
            auto deferred_chunks = ChunkMap{};
            auto chunk = chunk_factory.make_chunk(chunk_location);
            deferred_chunks.insert(chunk);

            // Plant trees
            if (chunk->location.z > ((float)KC::SEA_LEVEL / KC::CHUNK_SIZE))
            {
                auto deferred = chunk_mgr.plant_trees(chunk);
                deferred_chunks.insert(deferred.begin(), deferred.end());
            }

            for (const auto &deferred : deferred_chunks.values())
            {
                deferred->update_mesh();
            }
            chunk_mgr.GCL.insert(chunk);
        }

        auto end = std::chrono::high_resolution_clock::now();
        duration_ms = std::chrono::duration<float, std::milli>(end - start).count();
    }
    while (duration_ms < tgt_fps_ms);
}

struct AABB
{
    Vec3_t min;
    Vec3_t max;
};

static AABB make_player_aabb(const Vec3_t v_eye)
{
    // Weirdness with z is due to fact that camera sits at 1.5 of the players height (2)
    return {
        .min = { .v = { v_eye.x - 0.5f, v_eye.y - 0.5f, v_eye.z - 1.5f }},
        .max = { .v = { v_eye.x + 0.5f, v_eye.y + 0.5f, v_eye.z + 0.5f }}
    };
}

static AABB make_block_aabb(const Block &block)
{
    return {
        .min = { .v = {
            block.bottom_face.front().pos.x,
            block.bottom_face.front().pos.y,
            block.bottom_face.front().pos.z
        }},
        .max = { .v = {
            block.back_face.front().pos.x,
            block.back_face.front().pos.y,
            block.back_face.front().pos.z,
        }}
    };
}

static bool are_bodies_collided(const AABB &a, const AABB &b)
{
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

static void resolve_axis(
    Vec3_t &v_eye,
    int axis,
    std::shared_ptr<Chunk> chunk
)
{
    ChunkManager &chunk_mgr = ChunkManager::get_instance();
    Player &player = Player::get_instance();

    const float padding = 0.0001f;
    const Vec3_t chunk_world_location = { .v = {
        chunk->location.x * KC::CHUNK_SIZE,
        chunk->location.y * KC::CHUNK_SIZE,
        chunk->location.z * KC::CHUNK_SIZE
    }};

    AABB player_box = make_player_aabb(v_eye);

    // Find surrounding blocks from chunk that player might be colliding with
    int min_x = (int)roundf(player_box.min.x - chunk_world_location.x) - 1;
    int max_x = (int)roundf(player_box.max.x - chunk_world_location.x) + 1;

    int min_y = (int)roundf(player_box.min.y - chunk_world_location.y) - 1;
    int max_y = (int)roundf(player_box.max.y - chunk_world_location.y) + 1;

    int min_z = (int)roundf(player_box.min.z - chunk_world_location.z) - 1;
    int max_z = (int)roundf(player_box.max.z - chunk_world_location.z) + 1;

    // Check for collisions
    for (int z = min_z; z < max_z; ++z)
    {
        for (int y = min_y; y < max_y; ++y)
        {
            for (int x = min_x; x < max_x; ++x)
            {
                // Get proper coordinates since they might be outside current chunk
                Vec3_t actual_chunk{};
                Vec3_t actual_block{};

                actual_chunk.x = std::floorf(((chunk->location.x * KC::CHUNK_SIZE) + x) / KC::CHUNK_SIZE);
                actual_chunk.y = std::floorf(((chunk->location.y * KC::CHUNK_SIZE) + y) / KC::CHUNK_SIZE);
                actual_chunk.z = std::floorf(((chunk->location.z * KC::CHUNK_SIZE) + z) / KC::CHUNK_SIZE);

                actual_block.x = (((int)x % KC::CHUNK_SIZE) + KC::CHUNK_SIZE) % KC::CHUNK_SIZE;
                actual_block.y = (((int)y % KC::CHUNK_SIZE) + KC::CHUNK_SIZE) % KC::CHUNK_SIZE;
                actual_block.z = (((int)z % KC::CHUNK_SIZE) + KC::CHUNK_SIZE) % KC::CHUNK_SIZE;

                auto chunk = chunk_mgr.GCL.find(actual_chunk);
                if (chunk == nullptr)
                {
                    continue;
                }

                const Block &block = chunk->blocks[actual_block.z][actual_block.y][actual_block.x];
                if (block.type == BlockType::AIR)
                {
                    continue;
                }

                AABB block_box = make_block_aabb(block);
                if (!are_bodies_collided(player_box, block_box))
                {
                    continue;
                }

                // If collided, snap to the appropriate axis
                switch (axis)
                {
                    case 0:
                    {
                        if (player.v_vel.x > 0)
                        {
                            v_eye.x = block_box.min.x - 0.5f - padding;
                        }
                        else if (player.v_vel.x < 0)
                        {
                            v_eye.x = block_box.max.x + 0.5f + padding;
                        }
                        break;
                    }
                    case 1:
                    {
                        if (player.v_vel.y > 0)
                        {
                            v_eye.y = block_box.min.y - 0.5f - padding;
                        }
                        else if (player.v_vel.y < 0)
                        {
                            v_eye.y = block_box.max.y + 0.5f + padding;
                        }
                        break;
                    }
                    case 2:
                    {
                        float feet_offset = 1.5f;
                        float top_offset  = 0.5f;

                        if (player.v_vel.z > 0)
                        {
                            v_eye.z = block_box.min.z - top_offset - padding;
                        }
                        else if (player.v_vel.z < 0)
                        {
                            v_eye.z = block_box.max.z + feet_offset + padding;
                        }
                        break;
                    }
                }

                // Recalculate player bounding box after corrections
                player_box = make_player_aabb(v_eye);
            }
        }
    }
}

void Game::apply_physics(Camera &camera)
{
    ChunkManager &chunk_mgr = ChunkManager::get_instance();
    Player &player = Player::get_instance();

    Vec3_t plyr_chunk_location = { .v = {
        floorf(camera.v_eye.x / KC::CHUNK_SIZE),
        floorf(camera.v_eye.y / KC::CHUNK_SIZE),
        floorf(camera.v_eye.z / KC::CHUNK_SIZE)
    }};

    auto needle = chunk_mgr.GCL.find(plyr_chunk_location);
    if (needle == nullptr)
    {
        return;
    }

    // Resolve x-axis
    camera.v_eye.x += player.v_vel.x;
    resolve_axis(camera.v_eye, 0, needle);
    // Resolve y-axis
    camera.v_eye.y += player.v_vel.y;
    resolve_axis(camera.v_eye, 1, needle);
    // Resolve z-axis
    camera.v_eye.z += player.v_vel.z;
    resolve_axis(camera.v_eye, 2, needle);
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
    Player &player = Player::get_instance();

    while (XPending(kc_win.dpy) > 0)
    {
        // Blocks until event is received
        XNextEvent(kc_win.dpy, &kc_win.xev);

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
                    settings.is_running = false;
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
                camera.cast_ray();
                break;
            }
            default:
            {
                break;
            }
        }
    }

    // Update player movement
    Vec3_t v_fwd = camera.v_look_dir;
    Vec3_t v_right = qm_v3_norm(qm_v3_cross(KC::v_up, v_fwd));
    player.v_vel = {};

    if (IS_BIT_SET(key_mask, KeyAction::PLYR_FWD))
    {
        player.v_vel = qm_v3_add(player.v_vel, v_fwd);
    }
    if (IS_BIT_SET(key_mask, KeyAction::PLYR_BACK))
    {
        player.v_vel = qm_v3_sub(player.v_vel, v_fwd);
    }
    if (IS_BIT_SET(key_mask, KeyAction::PLYR_LEFT))
    {
        player.v_vel = qm_v3_add(player.v_vel, v_right);
    }
    if (IS_BIT_SET(key_mask, KeyAction::PLYR_RIGHT))
    {
        player.v_vel = qm_v3_sub(player.v_vel, v_right);
    }
    if (IS_BIT_SET(key_mask, KeyAction::PLYR_UP))
    {
        player.v_vel = qm_v3_add(player.v_vel, KC::v_up);
    }
    if (IS_BIT_SET(key_mask, KeyAction::PLYR_DOWN))
    {
        player.v_vel = qm_v3_sub(player.v_vel, KC::v_up);
    }

    player.v_vel = qm_v3_norm(player.v_vel);
    float magnitude = qm_v3_len(player.v_vel);
    if (magnitude > 0.0f)
    {
        player.v_vel = qm_v3_scale(player.v_vel, delta_time_ms * KC::PLAYER_SPEED_FACTOR);
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

    /*** Render terrain ***/

    block_shader.bind();

    // Model
    mvp.m_model = qm_m4_ident;
    u_model = glGetUniformLocation(block_shader.id, "model");
    glUniformMatrix4fv(u_model, 1, GL_TRUE, (float*)mvp.m_model.m);

    // View
    u_view = glGetUniformLocation(block_shader.id, "view");
    glUniformMatrix4fv(u_view, 1, GL_TRUE, (float*)mvp.m_view->m);

    // Projection
    mvp.m_proj = qm_m4_projection(
        settings.aspect,
        settings.fov,
        settings.znear,
        settings.zfar
    );
    u_proj = glGetUniformLocation(block_shader.id, "proj");
    glUniformMatrix4fv(u_proj, 1, GL_TRUE, (float*)mvp.m_proj.m);

    // Issue draw call
    glBindVertexArray(chunk_mgr.terrain_mesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, chunk_mgr.terrain_mesh.vertices.size());
    glBindVertexArray(0);

    block_shader.unbind();

    /*** Render skybox ***/

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    skybox_shader.bind();

    // Model
    mvp.m_model = qm_m4_translate(camera.v_eye.x, camera.v_eye.y, camera.v_eye.z);
    u_model = glGetUniformLocation(skybox_shader.id, "model");
    glUniformMatrix4fv(u_model, 1, GL_TRUE, (float*)mvp.m_model.m);

    // View
    u_view = glGetUniformLocation(skybox_shader.id, "view");
    glUniformMatrix4fv(u_view, 1, GL_TRUE, (float*)mvp.m_view->m);

    // Projection
    u_proj = glGetUniformLocation(skybox_shader.id, "proj");
    glUniformMatrix4fv(u_proj, 1, GL_TRUE, (float*)mvp.m_proj.m);

    // Issue draw call
    glBindVertexArray(skybox.mesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, skybox.mesh.vertices.size());
    glBindVertexArray(0);

    skybox_shader.unbind();

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    // Blit
    glFlush();

    // Update FPS thread
    fps++;
}
