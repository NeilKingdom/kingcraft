#pragma once

#include "common.hpp"
#include "mesh.hpp"

enum class BlockType
{
    AIR,
    DIRT,
    GRASS,
    SAND,
    WATER
};

class Block
{
public:
    BlockType   type;   // Block type
    Mesh        mesh;   // Contains the vertex attribute object of the block mesh
    vec3        pos;    // Block position in world space

    // Special member functions
    Block() = delete;
    Block(BlockType type)
    {
        this->type = type;
    }
    ~Block() = default;

    // General
    void render() const;
};
