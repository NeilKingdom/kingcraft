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

enum BlockFace : uint8_t
{
    RIGHT  = (1 << 0),
    LEFT   = (1 << 1),
    BACK   = (1 << 2),
    FRONT  = (1 << 3),
    BOTTOM = (1 << 4),
    TOP    = (1 << 5),
    ALL    = (RIGHT | LEFT | BACK | FRONT | BOTTOM | TOP)
};

class Block
{
public:
    BlockType type;
    uint8_t faces;

    std::array<VPosTex, 6> top_face;
    std::array<VPosTex, 6> bottom_face;
    std::array<VPosTex, 6> right_face;
    std::array<VPosTex, 6> left_face;
    std::array<VPosTex, 6> front_face;
    std::array<VPosTex, 6> back_face;

    // Special member functions
    Block();
    Block(const BlockType type, const uint8_t faces);
    ~Block() = default;
};
