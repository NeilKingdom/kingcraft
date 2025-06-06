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
 * @param[in] location A vec3 which determines the offset of the chunk relative to the world origin
 * @param[in] faces A bitmask representing the faces of the chunk to be rendered
 * @returns The constructed Chunk object
 */
std::shared_ptr<Chunk> ChunkFactory::make_chunk(const BlockFactory &block_factory, PerlinNoise &pn, const vec3 location, const bool is_tallest_in_col) const
{
    Settings &settings = Settings::get_instance();
    ssize_t chunk_size = settings.chunk_size;

    auto chunk = std::make_shared<Chunk>(Chunk(location));
    chunk->is_tallest_in_col = is_tallest_in_col;

    struct BlockData
    {
        uint8_t faces;
        BlockType type;
    } block_data;

    for (ssize_t y = -1; y < chunk_size + 1; ++y)
    {
        for (ssize_t x = -1; x < chunk_size + 1; ++x)
        {
            chunk->block_heights[y + 1][x + 1] = pn.octave_perlin(
                -(location[0] * chunk_size) + x,
                 (location[1] * chunk_size) + y,
                 0.8f, 0.05f, 3,
                 KC::SEA_LEVEL, KC::SEA_LEVEL + (chunk_size * 3)
            );
        }
    }

    for (ssize_t z = 0, _z = (location[2] * chunk_size); z < chunk_size; ++z, ++_z)
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

                vec3 block_location = {
                    -(location[0] * chunk_size) + x,
                     (location[1] * chunk_size) + y,
                     (location[2] * chunk_size) + z
                };

                // Construct block
                chunk->blocks[z][y][x] = block_factory.make_block(
                    block_data.type,
                    block_location,
                    block_data.faces
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
 * @param[in] location A vec2 specifying the (x, y) location of the chunk column to generate
 * @returns A vector of chunks that make up the chunk column
 */
std::vector<std::shared_ptr<Chunk>>
ChunkFactory::make_chunk_column(const BlockFactory &block_factory, PerlinNoise &pn, const vec2 location) const
{
    Settings &settings = Settings::get_instance();
    auto chunk_col = std::vector<std::shared_ptr<Chunk>>{};

    ssize_t chunk_size = settings.chunk_size;
    ssize_t height_lo = KC::MAX_BLOCK_HEIGHT;
    ssize_t height_hi = 0;
    ssize_t height;

    for (ssize_t y = 0; y < chunk_size; ++y)
    {
        for (ssize_t x = 0; x < chunk_size; ++x)
        {
            height = pn.octave_perlin(
                -(location[0] * chunk_size) + x,
                 (location[1] * chunk_size) + y,
                 0.8f, 0.05f, 3,
                 KC::SEA_LEVEL, KC::SEA_LEVEL + (chunk_size * 3)
            );

            if (height < height_lo)
            {
                height_lo = height;
            }

            if (height > height_hi)
            {
                height_hi = height;
            }
        }
    }

    // Solid terrain chunks
    for (size_t i = std::max(0, (int)(height_lo / chunk_size) - 1); i < (height_hi / chunk_size) + 1; ++i)
    {
        bool is_highest_in_col = (i == (height_hi / chunk_size));
        vec3 tmp_location = { location[0], location[1], (float)i };
        chunk_col.push_back(make_chunk(block_factory, pn, tmp_location, is_highest_in_col));
    }

    return chunk_col;
}

