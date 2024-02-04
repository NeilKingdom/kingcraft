#pragma once

#include "common.hpp"
#include "constants.hpp"

typedef struct _gameState
{
    // Game Logic-related variables
    struct 
    {
        bool isRunning = true;
    };

    // Camera-related variables
    struct 
    {
        float fov = 90.0f;
        float znear = 1.0f;
        float zfar = 1000.0f;
    };

    // Player-related variables
    struct 
    {
        float playerSpeed;
    };
} gameState;

