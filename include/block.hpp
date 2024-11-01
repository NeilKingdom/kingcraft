#pragma once

#include "common.hpp"
#include "mesh.hpp"

enum class BlockType
{
    AIR,
    DIRT,
    GRASS,
    STONE,
    SAND,
    WATER
};

struct Block
{
    BlockType   type;   // Block type
    Mesh        mesh;   // Contains the vertex attribute object of the block mesh

    // Special member functions
    Block() = delete;
    Block(BlockType type)
    {
        this->type = type;
    }
    ~Block() = default;
};
