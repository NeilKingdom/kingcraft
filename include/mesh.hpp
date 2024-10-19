#pragma once

struct Mesh
{
    unsigned vao;       // Vertex attribute object ID
    unsigned vbo;       // Vertex buffer object ID
    unsigned texture;   // Texture ID
    unsigned shader;    // Shader program ID
    unsigned vertices;  // Number of vertices that make up the mesh
};
