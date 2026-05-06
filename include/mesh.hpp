#pragma once

#include "common.hpp"

struct AttribPos
{
    float x;
    float y;
    float z;
};

struct AttribTex
{
    float u;
    float v;
};

struct AttribRgb
{
    float r;
    float g;
    float b;
};

struct Vertex
{
    AttribPos pos;
    AttribTex tex;
    AttribRgb rgb;
};

struct Mesh
{
    ID vao; // Vertex Attribute Object ID
    ID vbo; // Vertex Buffer Object ID
    std::vector<Vertex> vertices; // Vertex data
};
