#pragma once

#include "common.hpp"
#include "cube_renderer.hpp"

typedef struct 
{
    float x;
    float y;
    float z;
} Point;

class CubeFactory 
{
public:
    // TODO: Delete constructor and make this a singleton
    CubeFactory();
    ~CubeFactory();

private:  
    static CubeFactory cubeFactory;
};
