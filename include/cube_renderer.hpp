#pragma once

#include "common.hpp"
#include "cube_renderer.hpp"

struct Point
{
    float x;
    float y;
    float z;
};

class CubeFactory
{
public:
    // TODO: Delete constructor and make this a singleton
    CubeFactory() = delete;
    ~CubeFactory();

private:
    static const std::unique_ptr<CubeFactory> cubeFactory = std::make_shared<CubeFactory>();
};
