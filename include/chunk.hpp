#pragma once

#include "common.hpp"
#include "block.hpp"
#include "game_state.hpp"

// Forward declaration
class ChunkFactory;

class Chunk
{
public:
    vec3 location = {};
    std::vector<std::vector<std::vector<std::shared_ptr<Block>>>> blocks;

    // Special member functions
    Chunk() = default;
    ~Chunk() = default;

    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;

    // TODO: General
    //void add_block(const BlockType type, const uint8_t x, const uint8_t y, const uint8_t z);
    //void remove_block(const uint8_t x, const uint8_t y, const uint8_t z);

    // Operator overload
    bool operator==(const Chunk &chunk) const
    {
        return location[0] == chunk.location[0] && location[1] == chunk.location[1];
    }

private:
    friend ChunkFactory;
    //std::vector<std::vector<std::vector<uint8_t>>> m_block_faces;
};
