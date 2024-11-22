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
std::unique_ptr<Chunk> ChunkFactory::make_chunk(const mat4 &m_chunk_trns, const uint8_t faces) const
{
    auto chunk = std::make_unique<Chunk>();
    BlockFactory &block_factory = BlockFactory::get_instance();
    ssize_t chunk_size = GameState::get_instance().chunk_size;

    mat4 m_chunk_trns_idx = {};
    mat4 m_block_trns = {};
    mat4 m_location = {};

    assert(chunk_size > 1);

    struct BlockData
    {
        uint8_t faces;
        BlockType type;
    };

    chunk->location[0] = m_chunk_trns[3];
    chunk->location[1] = m_chunk_trns[7];

    std::vector<std::vector<std::vector<BlockData>>> tmp_data;
    tmp_data.resize(
        chunk_size,
        std::vector<std::vector<BlockData>>(
            chunk_size,
            std::vector<BlockData>(chunk_size)
        )
    );

    std::vector<std::vector<uint8_t>> heights;
    heights.resize(
        chunk_size,
        std::vector<uint8_t>(chunk_size)
    );

    for (ssize_t z = 0; z < chunk_size; ++z)
    {
        for (ssize_t y = 0; y < chunk_size; ++y)
        {
            for (ssize_t x = 0; x < chunk_size; ++x)
            {
                tmp_data[z][y][x].faces = 0;
                heights[y][x] = (std::rand() % (14 - 2 + 1)) + 2;
            }
        }
    }

    // Determine which faces to render for each block within the chunk
    for (ssize_t z = 0; z < chunk_size; ++z)
    {
        for (ssize_t y = 0; y < chunk_size; ++y)
        {
            for (ssize_t x = 0; x < chunk_size; ++x)
            {
                // Air block
                if (z > heights[y][x])
                {
                    tmp_data[z][y][x].type = BlockType::AIR;
                    continue;
                }

                // TODO: Determine block type based off z-value
                tmp_data[z][y][x].type = BlockType::GRASS;

                // Front
                if (x == 0 && IS_BIT_SET(faces, FRONT))
                {
                    tmp_data[z][y][x].faces |= FRONT;
                }
                else if (x > 0 && z > heights[y][x - 1])
                {
                    tmp_data[z][y][x].faces |= FRONT;
                }

                // Back
                if (x == (chunk_size - 1) && IS_BIT_SET(faces, BACK))
                {
                    tmp_data[z][y][x].faces |= BACK;
                }
                else if (x < (chunk_size - 1) && z > heights[y][x + 1])
                {
                    tmp_data[z][y][x].faces |= BACK;
                }

                // Left
                if (y == 0 && IS_BIT_SET(faces, LEFT))
                {
                    tmp_data[z][y][x].faces |= LEFT;
                }
                else if (y > 0 && z > heights[y - 1][x])
                {
                    tmp_data[z][y][x].faces |= LEFT;
                }

                // Right
                if (y == (chunk_size - 1) && IS_BIT_SET(faces, RIGHT))
                {
                    tmp_data[z][y][x].faces |= RIGHT;
                }
                else if (y < (chunk_size - 1) && z > heights[y + 1][x])
                {
                    tmp_data[z][y][x].faces |= RIGHT;
                }

                // Top
                if (z == (chunk_size - 1) && IS_BIT_SET(faces, TOP))
                {
                    tmp_data[z][y][x].faces |= TOP;
                }
                else if (z == heights[y][x])
                {
                    tmp_data[z][y][x].faces |= TOP;
                }

                // Bottom
                if (z == 0 && IS_BIT_SET(faces, BOTTOM))
                {
                    tmp_data[z][y][x].faces |= BOTTOM;
                }
            }
        }
    }

    // Create the actual Block objects for the chunk

    chunk->blocks.resize(chunk_size);
    for (ssize_t z = 0; z < chunk_size; ++z)
    {
        chunk->blocks[z].resize(chunk_size);
        for (ssize_t y = 0; y < chunk_size; ++y)
        {
            chunk->blocks[z][y].resize(chunk_size);
            for (ssize_t x = 0; x < chunk_size; ++x)
            {
                // Air block
                if (tmp_data[z][y][x].type == BlockType::AIR)
                {
                    chunk->blocks[z][y][x] = std::make_unique<Block>(Block(BlockType::AIR));
                    continue;
                }

                lac_get_translation_mat4(m_chunk_trns_idx, chunk->location[0] * chunk_size, chunk->location[1] * chunk_size, 1);
                lac_get_translation_mat4(m_block_trns, (float)x, (float)y, (float)z);

                // Block location = block position * chunk position index
                lac_multiply_mat4(m_location, m_block_trns, m_chunk_trns_idx);

                chunk->blocks[z][y][x] = block_factory.make_block(
                    tmp_data[z][y][x].type,
                    m_location,
                    tmp_data[z][y][x].faces
                );
            }
        }
    }

    // TODO: Remove
    std::cout << "Chunk pos: x = " << chunk->location[0] << ", y = " << chunk->location[1] << std::endl;

    return chunk;
}
