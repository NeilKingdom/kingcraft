#pragma once

#include "common.hpp"

struct BlockVertex
{
    float x;
    float y;
    float z;
    float u;
    float v;
};

struct SkyboxVertex
{
    float x;
    float y;
    float z;
};

template <typename T>
struct Mesh
{
    ID vao;                   // Vertex Attribute Object ID
    ID vbo;                   // Vertex Buffer Object ID
    std::vector<T> vertices;  // Vertex buffer data
};
