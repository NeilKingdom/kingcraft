/**
 * @file chunk_factory.cpp
 * @author Neil Kingdom
 * @since 16-10-2024
 * @version 1.0
 * @brief A factory class which constructs Chunk objects.
 */

#include "chunk_factory.hpp"

//static void defer(std::vector<DeferItem> &defer_list, const vec3 &chunk_location, const vec3 &block_location)
//{
//    vec3 actual_chunk_location{};
//    vec3 actual_block_location{};
//
//    actual_chunk_location[0] = std::floorf(((chunk_location[0] * 16) + block_location[0]) / 16.0f);
//    actual_chunk_location[1] = std::floorf(((chunk_location[1] * 16) + block_location[1]) / 16.0f);
//    actual_chunk_location[2] = std::floorf(((chunk_location[2] * 16) + block_location[2]) / 16.0f);
//
//    actual_block_location[0] = ((int)block_location[0] % 16 + 16) % 16;
//    actual_block_location[1] = ((int)block_location[1] % 16 + 16) % 16;
//    actual_block_location[2] = ((int)block_location[2] % 16 + 16) % 16;
//
//    defer_list.push_back(DeferItem{
//        std::array{ actual_chunk_location[0], actual_chunk_location[1], actual_chunk_location[2] },
//        std::array{ actual_block_location[0], actual_block_location[1], actual_block_location[2] }
//    });
//}

/**
 * @brief Creates a Chunk object given a set of input parameters.
 * @since 24-10-2024
 * @param[in] location A vec3 which determines the offset of the chunk relative to the world origin
 * @param[in] faces A bitmask representing the faces of the chunk to be rendered
 * @returns The constructed Chunk object
 */
std::shared_ptr<Chunk> ChunkFactory::make_chunk(PerlinNoise &pn, const vec3 location, const bool is_tallest_in_col) const
{
    BlockFactory block_factory;
    Settings &settings = Settings::get_instance();
    ssize_t chunk_size = settings.chunk_size;

    auto chunk = std::make_shared<Chunk>(Chunk(location));
    chunk->is_tallest_in_col = is_tallest_in_col;

    struct BlockData
    {
        uint8_t faces;
        BlockType type;
    };
    std::vector<std::vector<std::vector<BlockData>>> block_data;
    block_data.resize(
        chunk_size,
        std::vector<std::vector<BlockData>>(
            chunk_size,
            std::vector<BlockData>(chunk_size, BlockData{})
        )
    );

    std::vector<std::vector<uint8_t>> block_heights;
    block_heights.resize(chunk_size + 2, std::vector<uint8_t>(chunk_size + 2));
    for (ssize_t y = -1; y < chunk_size + 1; ++y)
    {
        for (ssize_t x = -1; x < chunk_size + 1; ++x)
        {
            block_heights[y + 1][x + 1] = pn.octave_perlin(
                -location[0] * chunk_size + x,
                 location[1] * chunk_size + y,
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
                if (_z > block_heights[_y][_x])
                {
                    continue;
                }

                // Determine block types
                // TODO: Add other block types at different z values
                if (x == 0)
                {
                    block_data[z][y][x].type = BlockType::SAND;
                }
                else
                {
                    block_data[z][y][x].type = BlockType::GRASS;
                }

                // Bottom
                if (_z == 0)
                {
                    block_data[z][y][x].faces |= BOTTOM;
                }

                // Top
                if (_z == block_heights[_y][_x])
                {
                    block_data[z][y][x].faces |= TOP;
                }

                // Front
                if (_z > block_heights[_y][_x - 1])
                {
                    block_data[z][y][x].faces |= FRONT;
                }

                // Back
                if (_z > block_heights[_y][_x + 1])
                {
                    block_data[z][y][x].faces |= BACK;
                }

                // Left
                if (_z > block_heights[_y - 1][_x])
                {
                    block_data[z][y][x].faces |= LEFT;
                }

                // Right
                if (_z > block_heights[_y + 1][_x])
                {
                    block_data[z][y][x].faces |= RIGHT;
                }

                vec3 block_location = {
                    -(location[0] * chunk_size) + x,
                     (location[1] * chunk_size) + y,
                     (location[2] * chunk_size) + z
                };

                // Construct block
                chunk->blocks[z][y][x] = block_factory.make_block(
                    block_data[z][y][x].type,
                    block_location,
                    block_data[z][y][x].faces
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
ChunkFactory::make_chunk_column(PerlinNoise &pn, const vec2 location) const
{
    Settings &settings = Settings::get_instance();
    auto chunk_col = std::vector<std::shared_ptr<Chunk>>{};

    ssize_t i;
    ssize_t chunk_size = settings.chunk_size;
    ssize_t height_lo = KC::MAX_BLOCK_HEIGHT;
    ssize_t height_hi = 0;
    ssize_t height;

    std::vector<std::vector<uint8_t>> heights;
    heights.resize(chunk_size, std::vector<uint8_t>(chunk_size));

    for (ssize_t y = 0; y < chunk_size; ++y)
    {
        for (ssize_t x = 0; x < chunk_size; ++x)
        {
            height = heights[y][x] = pn.octave_perlin(
                -location[0] * chunk_size + x,
                 location[1] * chunk_size + y,
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

    // Terrain chunks
    for (i = std::max(0, (int)(height_lo / chunk_size) - 1); i < (height_hi / chunk_size) + 1; ++i)
    {
        bool is_highest_in_col = (i == (height_hi / chunk_size));
        vec3 tmp_location = { location[0], location[1], (float)i };
        auto chunk = make_chunk(pn, tmp_location, is_highest_in_col);
        chunk_col.push_back(chunk);
    }

    // Empty air chunks
    for (; i < ((KC::SEA_LEVEL + (chunk_size * 3)) / chunk_size) + 1; ++i)
    {
        vec3 tmp_location = { location[0], location[1], (float)i };
        chunk_col.push_back(std::make_shared<Chunk>(Chunk(tmp_location)));
    }

    return chunk_col;
}

