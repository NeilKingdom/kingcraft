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
    ~Game();
    Game(const Game &game) = delete;
    Game &operator=(const Game &game) = delete;

private:
    GLXContext glx;
    KCWindow kc_win;
    KCWindow imgui_win;
    Player player;
    PerlinNoise pn;

    // General
    void generate_terrain();
    void plant_trees(ChunkManager &chunk_mgr, PerlinNoise &pn);
    void apply_physics();
    void process_events(KCWindow &win, Camera &camera);
    void render_frame(ChunkManager &chunk_mgr, Camera &camera, Mvp &mvp, KCShaders &shaders, SkyBox &skybox);
};
