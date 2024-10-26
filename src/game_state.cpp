/**
 * @file game_state.cpp
 * @author Neil Kingdom
 * @since 14-10-2024
 * @version 1.0
 * @brief A singleton class which acts as a context object for the game's state.
 */

#include "game_state.hpp"

/**
 * @brief Returns the single instance of GameState.
 * @since 14-10-2024
 * @returns The GameState instance
 */
GameState &GameState::get_instance()
{
    static GameState instance;
    return instance;
}

//void start() const {}
