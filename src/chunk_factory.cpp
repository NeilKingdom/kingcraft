/**
 * @file chunk_factory.cpp
 * @author Neil Kingdom
 * @since 16-10-2024
 * @version 1.0
 * @brief A singleton class which constructs a Chunk object.
 */

#include "chunk_factory.hpp"

/**
 * @brief Returns the single instance of ChunkFactory.
 * @since 24-10-2024
 * @returns The ChunkFactory instance
 */
ChunkFactory &ChunkFactory::get_instance()
{
    static ChunkFactory instance;
    return instance;
}

/**
 * @brief Creates a Chunk object given a set of input parameters.
 * @since 24-10-2024
 * @param[in] m_trns A 4x4 matrix which determines the offset of the chunk relative to the world origin
 * @param[in] faces A bitmask representing the faces of the chunk to be rendered
 * @returns The constructed Chunk object
 */
Chunk ChunkFactory::make_chunk(const mat4 m_trns, const uint8_t faces) const
{
    Chunk chunk;
    BlockFactory &block_factory = BlockFactory::get_instance();
    GameState &game = GameState::get_instance();
    mat4 m_blk_trns = {};

    // Determine which faces to render for each block within the chunk
    for (ssize_t z = 0; z < game.chunk_size; ++z)
    {
        for (ssize_t y = 0; y < game.chunk_size; ++y)
        {
            for (ssize_t x = 0; x < game.chunk_size; ++x)
            {
                if (x - 1 > 0)
                {
                    chunk.m_block_faces[z][y][x] &= ~BACK;
                    chunk.m_block_faces[z][y][x - 1] &= ~FRONT;
                }
                if (x + 1 < 16)
                {
                    chunk.m_block_faces[z][y][x] &= ~FRONT;
                    chunk.m_block_faces[z][y][x + 1] &= ~BACK;
                }
                if (y - 1 > 0)
                {
                    chunk.m_block_faces[z][y][x] &= ~LEFT;
                    chunk.m_block_faces[z][y - 1][x] &= ~RIGHT;
                }
                if (y + 1 < 16)
                {
                    chunk.m_block_faces[z][y][x] &= ~RIGHT;
                    chunk.m_block_faces[z][y + 1][x] &= ~LEFT;
                }
                if (z - 1 > 0)
                {
                    chunk.m_block_faces[z][y][x] &= ~BOTTOM;
                    chunk.m_block_faces[z - 1][y][x] &= ~TOP;
                }
                if (z + 1 < 16)
                {
                    chunk.m_block_faces[z][y][x] &= ~TOP;
                    chunk.m_block_faces[z + 1][y][x] &= ~BOTTOM;
                }
            }
        }
    }

    // Create the actual Block objects for the chunk
    for (size_t z = 0; z < game.chunk_size; ++z)
    {
        for (size_t y = 0; y < game.chunk_size; ++y)
        {
            for (size_t x = 0; x < game.chunk_size; ++x)
            {
                // TODO: Need to add m_trns (I think)...
                lac_get_translation_mat4(&m_blk_trns, (float)x, (float)y, (float)z);

                chunk.blocks[z][y][x] = block_factory.make_block(
                    chunk.m_block_types[z][y][x],
                    m_blk_trns,
                    (uint8_t)chunk.m_block_faces[z][y][x]
                );
            }
        }
    }

    return chunk;
}
