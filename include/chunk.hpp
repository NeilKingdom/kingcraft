#pragma once

#include "common.hpp"
#include "block.hpp"
#include "game_state.hpp"

// Forward declaration
class ChunkFactory;

class Chunk
{
public:
    Mesh mesh;
    vec3 location;
    uint8_t faces;
    std::vector<std::vector<std::vector<Block>>> blocks;

    // Special member functions
    Chunk() = default;
    ~Chunk();

    // TODO: General
    //void add_block(const BlockType type, const uint8_t x, const uint8_t y, const uint8_t z);
    //void remove_block(const uint8_t x, const uint8_t y, const uint8_t z);

    // Operator overloads

private:
    friend ChunkFactory;

    void flatten_block_data();
};
