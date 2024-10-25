#pragma once

#include "texture.hpp"

struct Mesh
{
    unsigned vao;           // Vertex attribute object ID
    unsigned vbo;           // Vertex buffer object ID
    unsigned vertices;      // Number of vertices that make up the mesh
    Texture texture;        // Texture of the mesh
};
