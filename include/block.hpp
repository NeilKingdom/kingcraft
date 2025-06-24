#pragma once

#include "common.hpp"
#include "mesh.hpp"

enum class BlockType
{
    AIR,
    DIRT,
    GRASS,
    WOOD,
    LEAVES,
    STONE,
    SAND,
    WATER
};

struct Block
{
    BlockType type;
    uint8_t faces;

    std::array<BlockVertex, 6> top_face;
    std::array<BlockVertex, 6> bottom_face;
    std::array<BlockVertex, 6> right_face;
    std::array<BlockVertex, 6> left_face;
    std::array<BlockVertex, 6> front_face;
    std::array<BlockVertex, 6> back_face;

    // Special member functions
    Block();
    Block(const BlockType type, const uint8_t faces);
    ~Block() = default;
};
