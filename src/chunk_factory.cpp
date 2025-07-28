/**
 * @file chunk_factory.cpp
 * @author Neil Kingdom
 * @since 16-10-2024
 * @version 1.0
 * @brief A factory class which constructs Chunk objects.
 */

#include "chunk_factory.hpp"

/**
 * @brief Creates a Chunk object given a set of input parameters.
 * @since 24-10-2024
 * TODO: params
 * @param[in] chunk_location A vec3 which determines the offset of the chunk relative to the world origin
 * @returns The constructed Chunk object
 */
std::shared_ptr<Chunk> ChunkFactory::make_chunk(
    const BlockFactory &block_factory,
    const PerlinNoise &pn,
    const vec3 chunk_location
) const
{
    auto start = std::chrono::high_resolution_clock::now();

    Settings &settings = Settings::get_instance();
    ssize_t chunk_size = settings.chunk_size;

    auto chunk = std::make_shared<Chunk>(chunk_location);

    struct BlockData
    {
        uint8_t faces;
        BlockType type;
    } block_data;

    // Calculate block heights
    for (ssize_t y = -1; y < chunk_size + 1; ++y)
    {
        for (ssize_t x = -1; x < chunk_size + 1; ++x)
        {
            chunk->block_heights[y + 1][x + 1] = pn.octave_perlin(
                -(chunk_location[0] * chunk_size) + x,
                 (chunk_location[1] * chunk_size) + y,
                 0.8f, 0.05f, 3,
                 KC::SEA_LEVEL, KC::SEA_LEVEL + (chunk_size * 3)
            );
        }
    }

    // Determine block types and visible faces
    for (ssize_t z = 0, _z = (chunk_location[2] * chunk_size); z < chunk_size; ++z, ++_z)
    {
        for (ssize_t y = 0, _y = 1; y < chunk_size; ++y, ++_y)
        {
            for (ssize_t x = 0, _x = 1; x < chunk_size; ++x, ++_x)
            {
                // Air blocks can be skipped
                if (_z > chunk->block_heights[_y][_x])
                {
                    continue;
                }

                // Determine block types
                // TODO: Add other block types at different z values
                if (x == 0)
                {
                    block_data.type = BlockType::SAND;
                }
                else if (y == 0)
                {
                    block_data.type = BlockType::STONE;
                }
                else
                {
                    block_data.type = BlockType::GRASS;
                }

                block_data.faces = 0;

                // Bottom
                if (_z == 0)
                {
                    block_data.faces |= BOTTOM;
                }

                // Top
                if (_z == chunk->block_heights[_y][_x])
                {
                    block_data.faces |= TOP;
                }

                // Front
                if (_z > chunk->block_heights[_y][_x - 1])
                {
                    block_data.faces |= FRONT;
                }

                // Back
                if (_z > chunk->block_heights[_y][_x + 1])
                {
                    block_data.faces |= BACK;
                }

                // Left
                if (_z > chunk->block_heights[_y - 1][_x])
                {
                    block_data.faces |= LEFT;
                }

                // Right
                if (_z > chunk->block_heights[_y + 1][_x])
                {
                    block_data.faces |= RIGHT;
                }

                vec3 world_location = {
                    -(chunk_location[0] * chunk_size) + x,
                     (chunk_location[1] * chunk_size) + y,
                     (chunk_location[2] * chunk_size) + z
                };

                // Construct block
                chunk->blocks[z][y][x] = block_factory.make_block(
                    block_data.type,
                    block_data.faces,
                    world_location
                );
            }
        }
    }

    chunk->update_mesh();
    return chunk;
}

/**
 * @brief Generates all chunks within a given chunk column.
 * @since 13-02-2025
 * TODO: params
 * @param[in] chunk_col_location A vec2 specifying the (x, y) location of the chunk column to generate
 * @returns A vector of chunks that make up the chunk column
 */
std::vector<Chunk> ChunkFactory::make_chunk_column(
    const BlockFactory &block_factory,
    const PerlinNoise &pn,
    const vec2 chunk_col_location
) const
{
    Settings &settings = Settings::get_instance();
    ssize_t chunk_size = settings.chunk_size;
    auto chunk_col = std::vector<Chunk>{};

    ssize_t min_block_height = KC::MAX_BLOCK_HEIGHT;
    ssize_t max_block_height = 0;
    ssize_t block_height;
    ssize_t min_chunk_height;
    ssize_t max_chunk_height;

    for (ssize_t y = 0; y < chunk_size; ++y)
    {
        for (ssize_t x = 0; x < chunk_size; ++x)
        {
            block_height = pn.octave_perlin(
                -(chunk_col_location[0] * chunk_size) + x,
                 (chunk_col_location[1] * chunk_size) + y,
                 0.8f, 0.05f, 3,
                 KC::SEA_LEVEL, KC::SEA_LEVEL + (chunk_size * 3)
            );

            if (block_height < min_block_height)
            {
                min_block_height = block_height;
            }

            if (block_height > max_block_height)
            {
                max_block_height = block_height;
            }
        }
    }

    min_chunk_height = std::max(0, (int)(min_block_height / chunk_size) - 1);
    max_chunk_height = (max_block_height / chunk_size) + 1;

    for (ssize_t i = min_chunk_height; i < max_chunk_height; ++i)
    {
        vec3 chunk_location = { chunk_col_location[0], chunk_col_location[1], (float)i };
        //chunk_col.push_back(make_chunk(block_factory, pn, chunk_location));
    }

    return chunk_col;
}

