#include "game_state.hpp"

GameState::GameState()
{}

GameState &GameState::get_instance()
{
    static GameState instance;
    return instance;
}

//void start() const {}
