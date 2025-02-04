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
 * @param[in] location A vec3 which determines the offset of the chunk relative to the world origin
 * @param[in] faces A bitmask representing the faces of the chunk to be rendered
 * @returns The constructed Chunk object
 */
std::shared_ptr<Chunk> ChunkFactory::make_chunk(const vec3 location, const uint8_t faces) const
{
    auto chunk = std::make_shared<Chunk>(Chunk());
    BlockFactory &block_factory = BlockFactory::get_instance();
    GameState &game = GameState::get_instance();
    ssize_t chunk_size = game.chunk_size;
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
            block_heights[y + 1][x + 1] = game.pn.octave_perlin(
                -location[0] * chunk_size + x,
                 location[1] * chunk_size + y,
                 0.8f,
                 0.05f, KC::SEA_LEVEL, KC::SEA_LEVEL + (chunk_size * 3)
            );
        }
    }

    chunk->blocks.resize(chunk_size);
    for (ssize_t _z = (location[2] * chunk_size), z = 0; _z < (location[2] * chunk_size) + chunk_size; ++_z, z = _z - (location[2] * chunk_size))
    {
        chunk->blocks[z].resize(chunk_size);
        for (ssize_t _y = 1, y = 0; _y < chunk_size + 1; ++_y, y = _y - 1)
        {
            chunk->blocks[z][y].resize(chunk_size);
            for (ssize_t _x = 1, x = 0; _x < chunk_size + 1; ++_x, x = _x - 1)
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

                // Construct block
                chunk->blocks[z][y][x] = block_factory.make_block(
                    block_data[z][y][x].type,
                    vec3{
                        -(location[0] * chunk_size) + x,
                         (location[1] * chunk_size) + y,
                         (location[2] * chunk_size) + z
                    },
                    block_data[z][y][x].faces
                );
            }
        }
    }

    chunk->squash_block_meshes();
    return chunk;
}

std::vector<std::shared_ptr<Chunk>> ChunkFactory::make_chunk_column(const vec2 location) const
{
    GameState &game = GameState::get_instance();
    ChunkFactory &chunk_factory = ChunkFactory::get_instance();
    auto chunk_col = std::vector<std::shared_ptr<Chunk>>{};

    ssize_t chunk_size = game.chunk_size;
    ssize_t height_lo = KC::MAX_BLOCK_HEIGHT;
    ssize_t height_hi = 0;
    ssize_t height;

    std::vector<std::vector<uint8_t>> heights;
    heights.resize(chunk_size, std::vector<uint8_t>(chunk_size));

    for (ssize_t y = 0; y < chunk_size; ++y)
    {
        for (ssize_t x = 0; x < chunk_size; ++x)
        {
            height = heights[y][x] = game.pn.octave_perlin(
                -location[0] * chunk_size + x,
                 location[1] * chunk_size + y,
                 0.8f,
                 0.05f, KC::SEA_LEVEL, KC::SEA_LEVEL + (chunk_size * 3)
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

    for (int i = std::max(0, (int)(height_lo / chunk_size) - 1); i < (height_hi / chunk_size) + 1; ++i)
    {
        vec3 tmp_location = { location[0], location[1], (float)i };
        chunk_col.push_back(make_chunk(tmp_location, ALL));
    }

    return chunk_col;
}
