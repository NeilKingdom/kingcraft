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
    BlockType type;  // Block type
    Mesh      mesh;  // Contains the vertex attribute object of the block mesh
    uint8_t   faces; // Current block faces

    // Special member functions
    Block() = delete;
    Block(BlockType type);
    ~Block();

    // General purpose
    void add_face(uint8_t face);
    void remove_face(uint8_t face);

private:
    void modify_face();
};
