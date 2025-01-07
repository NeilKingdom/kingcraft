#pragma once

#include "common.hpp"

struct Mesh
{
    ID vao;                         // Vertex attribute object ID
    ID vbo;                         // Vertex buffer object ID
    std::vector<float> vertices;    // Raw buffer for vertex data
};
