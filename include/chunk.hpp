#pragma once

#include "common.hpp"
#include "block.hpp"
#include "game_state.hpp"

// Forward declaration
class ChunkFactory;

struct Chunk
{
    std::vector<std::vector<std::vector<Block>>> blocks;

    // Special member functions
    Chunk()
    {
        GameState &game = GameState::get_instance();

        blocks.reserve(game.chunk_size);

        for (size_t i = 0; i < game.chunk_size; ++i)
        {
            blocks.emplace_back();
            blocks[i].reserve(game.chunk_size);

            for (size_t j = 0; j < game.chunk_size; ++j)
            {
                blocks[i].emplace_back();
                blocks[i][j].reserve(game.chunk_size);
            }
        }

        m_block_faces = std::vector(
            game.chunk_size,
            std::vector<std::vector<uint8_t>>(
                game.chunk_size,
                std::vector<uint8_t>(game.chunk_size)
            )
        );
        m_block_types = std::vector(
            game.chunk_size,
            std::vector<std::vector<BlockType>>(
                game.chunk_size,
                std::vector<BlockType>(game.chunk_size)
            )
        );

        for (size_t z = 0; z < game.chunk_size; ++z)
        {
            for (size_t y = 0; y < game.chunk_size; ++y)
            {
                for (size_t x = 0; x < game.chunk_size; ++x)
                {
                    m_block_faces[z][y][x] = ALL;
                    // TODO: Determine block type based off y value
                    m_block_types[z][y][x] = BlockType::GRASS;
                }
            }
        }
    }
    ~Chunk() = default;

private:
    friend ChunkFactory;

    std::vector<std::vector<std::vector<uint8_t>>>   m_block_faces;
    std::vector<std::vector<std::vector<BlockType>>> m_block_types;
};
