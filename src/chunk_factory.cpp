/**
 * @file chunk_factory.cpp
 * @author Neil Kingdom
 * @since 16-10-2024
 * @version 1.0
 * @brief A factory class which constructs Chunk objects.
 */

#include "chunk_factory.hpp"

/**
 * @brief Produces a pseudo-random hash based off __location__ and maps it to the range __min__..__max__.
 * @since 13-02-2025
 * @param[in] location A vec3, from which the hash is produced
 * @param[in] min The minimum value that can be produced
 * @param[in] max The maximum value that can be produced
 * @returns A pseudo-random number in the range __min__..__max__
 */
size_t ChunkFactory::hash_coord_to_range(const vec3 location, const size_t min, const size_t max) const
{
    size_t hx = std::hash<int>{}(location[0]);
    size_t hy = std::hash<int>{}(location[1]);
    size_t hz = std::hash<int>{}(location[2]);
    size_t hash = hx ^ (hy << 1) ^ (hz << 2);
    return hash % ((max - min + 1) + min);
}

/**
 * @brief Creates a Chunk object given a set of input parameters.
 * @since 24-10-2024
 * @param[in] location A vec3 which determines the offset of the chunk relative to the world origin
 * @param[in] faces A bitmask representing the faces of the chunk to be rendered
 * @returns The constructed Chunk object
 */
std::shared_ptr<Chunk> ChunkFactory::make_chunk(PerlinNoise &pn, const vec3 location, const uint8_t faces) const
{
    BlockFactory block_factory;
    Settings &settings = Settings::get_instance();
    auto chunk = std::make_shared<Chunk>(Chunk());

    ssize_t chunk_size = settings.chunk_size;
    assert(chunk_size > 1);

    std::memcpy(chunk->location, location, sizeof(vec3));
    chunk->faces = faces;

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

    chunk->blocks.resize(
        chunk_size,
        std::vector<std::vector<Block>>(
            chunk_size,
            std::vector<Block>(chunk_size, Block())
        )
    );

    for (ssize_t z = 0, _z = (location[2] * chunk_size); z < chunk_size; ++z, ++_z)
    {
        for (ssize_t y = 0, _y = 1; y < chunk_size; ++y, ++_y)
        {
            for (ssize_t x = 0, _x = 1; x < chunk_size; ++x, ++_x)
            {
                // Determine block types
                // TODO: Add other block types at different z values
                if (_z > block_heights[_y][_x])
                {
                    block_data[z][y][x].type = BlockType::AIR;
                    continue;
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

                // Pseudo-random chance to plant tree here (based off block hash)
                //if (_z == block_heights[_y][_x] && hash_coord_to_range(block_location, 0, 20) == 0)
                //{
                //    ChunkFactory::plant_tree(
                //        chunk,
                //        vec3{ (float)x, (float)y, (float)(_z % 16) }
                //    );
                //}
            }
        }
    }

    chunk->squash_block_meshes();
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

    for (ssize_t i = std::max(0, (int)(height_lo / chunk_size) - 1); i < (height_hi / chunk_size) + 1; ++i)
    {
        vec3 tmp_location = { location[0], location[1], (float)i };
        chunk_col.push_back(make_chunk(pn, tmp_location, ALL));
    }

    return chunk_col;
}
