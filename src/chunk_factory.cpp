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
Chunk ChunkFactory::make_chunk(const mat4 &m_trns, const uint8_t faces) const
{
    Chunk chunk = Chunk();
    BlockFactory &block_factory = BlockFactory::get_instance();
    GameState &game = GameState::get_instance();

    ssize_t chunk_size = game.chunk_size;
    mat4 m_blk_trns = {};

    // Determine which faces to render for each block within the chunk
    for (ssize_t z = 0; z < chunk_size; ++z)
    {
        for (ssize_t y = 0; y < chunk_size; ++y)
        {
            for (ssize_t x = 0; x < chunk_size; ++x)
            {
                // Check if on the exterior of the chunk
                if (chunk.blocks[z][y][x].type != BlockType::AIR)
                {
                    if (x == 0 && IS_BIT_SET(faces, FRONT))
                    {
                        chunk.m_block_faces[z][y][x] |= FRONT;
                    }
                    if (x == chunk_size - 1 && IS_BIT_SET(faces, BACK))
                    {
                        chunk.m_block_faces[z][y][x] |= BACK;
                    }
                    if (y == 0 && IS_BIT_SET(faces, LEFT))
                    {
                        chunk.m_block_faces[z][y][x] |= LEFT;
                    }
                    if (y == chunk_size - 1 && IS_BIT_SET(faces, RIGHT))
                    {
                        chunk.m_block_faces[z][y][x] |= RIGHT;
                    }
                    if (z == 0 && IS_BIT_SET(faces, BOTTOM))
                    {
                        chunk.m_block_faces[z][y][x] |= BOTTOM;
                    }
                    if (z == chunk_size - 1 && IS_BIT_SET(faces, TOP))
                    {
                        chunk.m_block_faces[z][y][x] |= TOP;
                    }
                }
                // Make neighboring block faces visible
                else
                {
                    if (x > 0)
                    {
                        chunk.m_block_faces[z][y][x - 1] |= BACK;
                    }
                    if (x < chunk_size - 1)
                    {
                        chunk.m_block_faces[z][y][x + 1] |= FRONT;
                    }
                    if (y > 0)
                    {
                        chunk.m_block_faces[z][y - 1][x] |= RIGHT;
                    }
                    if (y < chunk_size - 1)
                    {
                        chunk.m_block_faces[z][y + 1][x] |= LEFT;
                    }
                    if (z > 0)
                    {
                        chunk.m_block_faces[z - 1][y][x] |= TOP;
                    }
                    if (z < chunk_size - 1)
                    {
                        chunk.m_block_faces[z + 1][y][x] |= BOTTOM;
                    }
                }
            }
        }
    }

    // Create the actual Block objects for the chunk
    for (ssize_t z = 0; z < chunk_size; ++z)
    {
        for (ssize_t y = 0; y < chunk_size; ++y)
        {
            for (ssize_t x = 0; x < chunk_size; ++x)
            {
                // TODO: Need to add m_trns (I think)...
                mat4 tmp = {};
                lac_get_translation_mat4(&m_blk_trns, (float)x, (float)y, (float)(z - chunk_size));
                lac_multiply_mat4(&tmp, m_trns, m_blk_trns);

                chunk.blocks[z][y][x] = block_factory.make_block(
                    chunk.blocks[z][y][x].type,
                    tmp,
                    chunk.m_block_faces[z][y][x]
                );
            }
        }
    }

    return chunk;
}
