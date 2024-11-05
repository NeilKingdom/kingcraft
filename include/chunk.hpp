#pragma once

#include "common.hpp"
#include "block.hpp"
#include "game_state.hpp"

// Forward declaration
class ChunkFactory;

class Chunk
{
public:
    std::vector<std::vector<std::vector<Block>>> blocks;

    // Special member functions
    Chunk() : m_location{}
    {
        GameState &game = GameState::get_instance();

        m_block_faces.resize(
            game.chunk_size,
            std::vector<std::vector<uint8_t>>(
                game.chunk_size,
                std::vector<uint8_t>(game.chunk_size, 0)
            )
        );
    }
    ~Chunk() = default;

    // General
    void add_block(const BlockType type, const uint8_t x, const uint8_t y, const uint8_t z)
    {}
    void remove_block(const uint8_t x, const uint8_t y, const uint8_t z)
    {}

private:
    friend ChunkFactory;

    vec3 m_location;
    std::vector<std::vector<std::vector<uint8_t>>> m_block_faces;
};
