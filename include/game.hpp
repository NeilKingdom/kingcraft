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
        Settings &settings,
        ChunkManager &chunk_mgr,
        Camera &camera,
        ChunkFactory &chunk_factory,
        BlockFactory &block_factory,
        Mvp &mvp,
        KCShaders &shaders,
        SkyBox &skybox
    );
    void plant_trees(
        Camera &camera,
        const BlockFactory &block_factory,
        Mvp &mvp,
        KCShaders &shaders,
        SkyBox &skybox
    );
    void apply_physics();
    void process_events(Settings &settings, Camera &camera);
    void render_frame(ChunkManager &chunk_mgr, Camera &camera, Mvp &mvp, KCShaders &shaders, SkyBox &skybox);
    void cleanup();
};
