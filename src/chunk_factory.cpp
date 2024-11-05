/**
 * @file chunk_factory.cpp
 * @author Neil Kingdom
 * @since 16-10-2024
 * @version 1.0
 * @brief A singleton class which constructs Chunk objects.
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
 * @param[in] m_chunk_trns A 4x4 matrix which determines the offset of the chunk relative to the world origin
 * @param[in] faces A bitmask representing the faces of the chunk to be rendered
 * @returns The constructed Chunk object
 */
Chunk ChunkFactory::make_chunk(const mat4 &m_chunk_trns, const uint8_t faces) const
{
    Chunk chunk = Chunk();
    BlockFactory &block_factory = BlockFactory::get_instance();
    ssize_t chunk_size = GameState::get_instance().chunk_size;

    mat4 m_block_trns = {};
    mat4 m_location = {};

    // Determine which faces to render for each block within the chunk
    for (ssize_t z = 0; z < chunk_size; ++z)
    {
        for (ssize_t y = 0; y < chunk_size; ++y)
        {
            for (ssize_t x = 0; x < chunk_size; ++x)
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
        }
    }

    // Create the actual Block objects for the chunk

    chunk.blocks.resize(chunk_size);
    for (ssize_t z = 0; z < chunk_size; ++z)
    {
        chunk.blocks[z].resize(chunk_size);
        for (ssize_t y = 0; y < chunk_size; ++y)
        {
            for (ssize_t x = 0; x < chunk_size; ++x)
            {
                // Block location = chunk's translation matrix * the block's translation matrix
                lac_get_translation_mat4(&m_block_trns, (float)x, (float)y, (float)(z - chunk_size));
                lac_multiply_mat4(&m_location, m_chunk_trns, m_block_trns);

                // TODO: Determine block type by z coordinate
                chunk.blocks[z][y].push_back(
                    block_factory.make_block(
                        BlockType::GRASS,
                        m_location,
                        chunk.m_block_faces[z][y][x]
                    )
                );
            }
        }
    }

    return chunk;
}
