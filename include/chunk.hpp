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
    Chunk()
    {
        GameState &game = GameState::get_instance();

        blocks.resize(
            game.chunk_size,
            std::vector<std::vector<Block>>(
                game.chunk_size,
                // TODO: Determine block type based off z coordinates
                std::vector<Block>(game.chunk_size, Block(BlockType::GRASS))
            )
        );

        m_block_faces.resize(
            game.chunk_size,
            std::vector<std::vector<uint8_t>>(
                game.chunk_size,
                std::vector<uint8_t>(
                    game.chunk_size, 0)
            )
        );
    }
    ~Chunk() = default;

private:
    friend ChunkFactory;

    std::vector<std::vector<std::vector<uint8_t>>> m_block_faces;
};
