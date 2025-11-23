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
#include "utils.hpp"

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
    std::thread fps_thread; // Thread for tracking game FPS

    // TODO: Not a fan of having these here...
    Shader block_shader;
    Shader skybox_shader;

    // General
    void generate_terrain(Camera &camera);
    ChunkSet plant_trees(std::shared_ptr<Chunk> &chunk);
    void apply_physics();
    void process_events(Camera &camera);
    void render_frame(Camera &camera, Mvp &mvp, SkyBox &skybox);
    void cleanup();
};
