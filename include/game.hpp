#pragma once

#include "common.hpp"
#include "camera.hpp"
#include "constants.hpp"
#include "window.hpp"
#include "settings.hpp"
#include "shader.hpp"
#include "chunk_factory.hpp"
#include "chunk_manager.hpp"
#include "skybox.hpp"
#include "player.hpp"
#include "perlin_noise.hpp"
#include "mvp.hpp"
#include "helpers.hpp"

class Game
{
public:
    // Special member functions
    Game();
    ~Game() = default;
    Game(const Game &game) = delete;
    Game &operator=(const Game &game) = delete;

private:
    GLXContext glx; // OpenGL Context
    KCWindow kc_win; // KingCraft window
    KCWindow imgui_win; // ImGui window
    Player player; // Player instance
    PerlinNoise pn; // Perlin noise instance
    std::thread fps_thread; // Thread for tracking game FPS

    // General
    void generate_terrain(
        ChunkManager &chunk_mgr,
        ChunkFactory &chunk_factory,
        BlockFactory &block_factory,
        Camera &camera,
        Settings &settings
    );
    void plant_trees(
        ChunkManager &chunk_mgr,
        BlockFactory &block_factory,
        Camera &camera,
        Mvp &mvp,
        KCShaders &shaders,
        SkyBox &skybox,
        Settings &settings
    );
    void apply_physics();
    void process_events(Camera &camera, Settings &settings);
    void render_frame(
        ChunkManager &chunk_mgr,
        Camera &camera,
        Mvp &mvp,
        KCShaders &shaders,
        SkyBox &skybox,
        Settings &settings
    );
    void cleanup();
};
