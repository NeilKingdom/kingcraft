#pragma once

#include "texture.hpp"
#include "shader_program.hpp"

struct Mesh
{
    unsigned vao;           // Vertex attribute object ID
    unsigned vbo;           // Vertex buffer object ID
    unsigned vertices;      // Number of vertices that make up the mesh
    Texture texture;        // Texture of the mesh
    ShaderProgram shader;   // Shader used for rendering the mesh
};
