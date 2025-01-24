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

class Block
{
public:
    BlockType type;
    uint8_t faces;
    std::vector<float> vertices;

    // Special member functions
    Block();
    Block(BlockType type);
    ~Block() = default;
};
