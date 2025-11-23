/**
 * @file chunk_factory.cpp
 * @author Neil Kingdom
 * @since 16-10-2024
 * @version 1.0
 * @brief A factory class which constructs Chunk objects.
 */

#include "chunk_factory.hpp"

ChunkFactory &ChunkFactory::get_instance()
{
    static ChunkFactory chunk_factory;
    return chunk_factory;
}

/**
 * @brief Creates a Chunk object given a set of input parameters.
 * @since 24-10-2024
 * TODO: params
 * @param[in] chunk_location A vec3 which determines the offset of the chunk relative to the world origin
 * @returns The constructed Chunk object
 */
std::shared_ptr<Chunk> ChunkFactory::make_chunk(const vec3 chunk_location) const
{
    BlockFactory &block_factory = BlockFactory::get_instance();

    auto chunk = std::make_shared<Chunk>(chunk_location);

    struct BlockData
    {
        uint8_t faces;
        BlockType type;
    } block_data;

    // Calculate block heights
    for (ssize_t y = -1; y < KC::CHUNK_SIZE + 1; ++y)
    {
        for (ssize_t x = -1; x < KC::CHUNK_SIZE + 1; ++x)
        {
            chunk->block_heights[y + 1][x + 1] = sample_biome_height(
                vec2{
                    (chunk_location[0] * KC::CHUNK_SIZE) + x,
                    (chunk_location[1] * KC::CHUNK_SIZE) + y
                }
            );
        }
    }

    // Determine block types and visible faces
    for (ssize_t z = 0, _z = (chunk_location[2] * KC::CHUNK_SIZE); z < KC::CHUNK_SIZE; ++z, ++_z)
    {
        for (ssize_t y = 0, _y = 1; y < KC::CHUNK_SIZE; ++y, ++_y)
        {
            for (ssize_t x = 0, _x = 1; x < KC::CHUNK_SIZE; ++x, ++_x)
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

                if (x == 1 && y == 1)
                {
                    block_data.type = BlockType::DIRT;
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
                     (chunk_location[0] * KC::CHUNK_SIZE) + x,
                     (chunk_location[1] * KC::CHUNK_SIZE) + y,
                     (chunk_location[2] * KC::CHUNK_SIZE) + z
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

    return chunk;
}
