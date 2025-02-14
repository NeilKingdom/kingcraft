#pragma once

#include "common.hpp"

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

    // TODO: Ew, Magic numbers!
    std::array<float, 6 * 5> top_face;
    std::array<float, 6 * 5> bottom_face;
    std::array<float, 6 * 5> right_face;
    std::array<float, 6 * 5> left_face;
    std::array<float, 6 * 5> front_face;
    std::array<float, 6 * 5> back_face;

    // Special member functions
    Block();
    Block(const BlockType type);
    ~Block() = default;
};
