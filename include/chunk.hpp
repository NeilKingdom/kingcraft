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

    // General
    void add_block(const BlockType type, const vec3 location);
    void remove_block(const vec3 location);

    // Operator overloads
    bool operator==(const Chunk &chunk) const;

private:
    friend ChunkFactory;

    void flatten_block_data();
};
