#pragma once

#include "common.hpp"
#include "player.hpp"

class GameState
{
public:
    float fov = 90.0f;
    float aspect = 9.0f / 16.0f;
    float znear = 1.0f;
    float zfar = 1000.0f;

    size_t chunk_size = 16;

    bool is_running = true;

    Player player = Player();

    // Special member functions
    GameState(const GameState&) = delete;
    GameState &operator=(const GameState&) = delete;
    ~GameState() = default;

    // General
    static GameState &get_instance();
    //void start() const;

private:
    // Special member functions
    GameState() = default;
};
