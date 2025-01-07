#pragma once

#include "common.hpp"
#include "player.hpp"
#include "perlin_noise.hpp"

class GameState
{
public:
    float fov    = 89.0f;
    float aspect = 16.0f / 9.0f;
    float znear  = 1.0f;
    float zfar   = 1000.0f;

    size_t chunk_size = 16;

    unsigned long seed = 12345L; // TODO: Temporarily hard-coded
    bool is_running = true;

    Player player = Player();
    PerlinNoise pn = PerlinNoise();

    // Special member functions
    GameState(const GameState &game) = delete;
    GameState &operator=(const GameState &game) = delete;
    ~GameState() = default;

    // General
    static GameState &get_instance();
    //void start() const;

private:
    // Special member functions
    GameState() = default;
};
