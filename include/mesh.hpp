#pragma once

#include "common.hpp"

struct VPos
{
    float x;
    float y;
    float z;
};

struct VPosTex
{
    float x;
    float y;
    float z;
    float u;
    float v;
};

struct VPosColTex
{
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float a;
    float u;
    float v;
};

template <typename T>
struct Mesh
{
    ID vao;                   // Vertex Attribute Object ID
    ID vbo;                   // Vertex Buffer Object ID
    std::vector<T> vertices;  // Vertex buffer data
};
