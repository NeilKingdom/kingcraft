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
Chunk ChunkFactory::make_chunk(const vec3 location, const uint8_t faces) const
{
    auto chunk = Chunk();
    BlockFactory &block_factory = BlockFactory::get_instance();
    ssize_t chunk_size = GameState::get_instance().chunk_size;
    assert(chunk_size > 1);

    std::memcpy(chunk.location, location, sizeof(vec3));

    struct BlockData
    {
        uint8_t faces;
        BlockType type;
    };

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

    const float scale = 0.05f;

    for (ssize_t z = 0; z < chunk_size; ++z)
    {
        for (ssize_t y = 0; y < chunk_size; ++y)
        {
            for (ssize_t x = 0; x < chunk_size; ++x)
            {
                tmp_data[z][y][x].faces = 0;
                heights[y][x] = octave_perlin(
                    -location[0] * chunk_size + x,
                    location[1] * chunk_size + y,
                    2, scale, 0, 15
                );
                //heights[y][x] = 15;
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
                    SET_BIT(chunk.faces, FRONT);
                }
                else if (x > 0 && z > heights[y][x - 1])
                {
                    tmp_data[z][y][x].faces |= FRONT;
                    SET_BIT(chunk.faces, FRONT);
                }

                // Back
                if (x == (chunk_size - 1) && IS_BIT_SET(faces, BACK))
                {
                    tmp_data[z][y][x].faces |= BACK;
                    SET_BIT(chunk.faces, BACK);
                }
                else if (x < (chunk_size - 1) && z > heights[y][x + 1])
                {
                    tmp_data[z][y][x].faces |= BACK;
                    SET_BIT(chunk.faces, BACK);
                }

                // Left
                if (y == 0 && IS_BIT_SET(faces, LEFT))
                {
                    tmp_data[z][y][x].faces |= LEFT;
                    SET_BIT(chunk.faces, LEFT);
                }
                else if (y > 0 && z > heights[y - 1][x])
                {
                    tmp_data[z][y][x].faces |= LEFT;
                    SET_BIT(chunk.faces, LEFT);
                }

                // Right
                if (y == (chunk_size - 1) && IS_BIT_SET(faces, RIGHT))
                {
                    tmp_data[z][y][x].faces |= RIGHT;
                    SET_BIT(chunk.faces, RIGHT);
                }
                else if (y < (chunk_size - 1) && z > heights[y + 1][x])
                {
                    tmp_data[z][y][x].faces |= RIGHT;
                    SET_BIT(chunk.faces, RIGHT);
                }

                // Top
                if (z == (chunk_size - 1) && IS_BIT_SET(faces, TOP))
                {
                    tmp_data[z][y][x].faces |= TOP;
                    SET_BIT(chunk.faces, TOP);
                }
                else if (z == heights[y][x])
                {
                    tmp_data[z][y][x].faces |= TOP;
                    SET_BIT(chunk.faces, TOP);
                }

                // Bottom
                if (z == 0 && IS_BIT_SET(faces, BOTTOM))
                {
                    tmp_data[z][y][x].faces |= BOTTOM;
                    SET_BIT(chunk.faces, BOTTOM);
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
            chunk.blocks[z][y].resize(chunk_size);
            for (ssize_t x = 0; x < chunk_size; ++x)
            {
                chunk.blocks[z][y][x] = block_factory.make_block(
                    tmp_data[z][y][x].type,
                    vec3{
                        -(location[0] * chunk_size) + x,
                         (location[1] * chunk_size) + y,
                         (location[2] * chunk_size) + z
                    },
                    tmp_data[z][y][x].faces
                );
            }
        }
    }

    chunk.flatten_block_data();
    return chunk;
}
