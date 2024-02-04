#pragma once

#include "common.hpp"
#include "constants.hpp"

typedef struct _gameState
{
    // Projection variables
    struct 
    {
        float fov = lac_deg_to_rad(90.0f);
        float znear = 1.0f;
        float zfar = 1000.0f;
    };

    // Player-related variables
    struct 
    {
        float playerSpeed;
    };
} gameState;

