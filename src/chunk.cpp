/**
 * @file chunk.cpp
 * @author Neil Kingdom
 * @since 14-06-2024
 * @version 1.0
 * @brief Contains operations and helper functions pertaining to Chunk objects.
 */

#include "chunk.hpp"

Chunk::Chunk()
    : updated(false), is_tallest_in_col(false)
{
    Settings &settings = Settings::get_instance();
    ssize_t chunk_size = settings.chunk_size;

    blocks.resize(
        chunk_size,
        std::vector<std::vector<Block>>(
            chunk_size,
            std::vector<Block>(chunk_size, Block())
        )
    );
}

Chunk::Chunk(const vec3 location)
    : updated(false), is_tallest_in_col(false)
{
    Settings &settings = Settings::get_instance();
    ssize_t chunk_size = settings.chunk_size;

    std::memcpy(this->location, location, sizeof(vec3));
    blocks.resize(
        chunk_size,
        std::vector<std::vector<Block>>(
            chunk_size,
            std::vector<Block>(chunk_size, Block())
        )
    );
}

/**
 * @brief Operator overload for equality operation.
 * Chunks are considered to be equal if their positions match.
 * @since 13-02-2025
 * @param[in] chunk The chunk to compare against
 * @returns True if the chunks match, otherwise returns false
 */
bool Chunk::operator==(const Chunk &chunk) const
{
    return is_equal(this->location, chunk.location);
}

/**
 * @brief Squashes the block vertices into one unified mesh.
 * @since 13-02-2025
 */
void Chunk::update_mesh()
{
    Settings &settings = Settings::get_instance();
    ssize_t chunk_size = settings.chunk_size;

    updated = true;
    vertices.clear();

    for (ssize_t z = 0; z < chunk_size; ++z)
    {
        for (ssize_t y = 0; y < chunk_size; ++y)
        {
            for (ssize_t x = 0; x < chunk_size; ++x)
            {
                Block &block = blocks[z][y][x];
                if (block.type != BlockType::AIR && block.faces != 0)
                {
                    if (IS_BIT_SET(block.faces, BOTTOM))
                    {
                        vertices.insert(vertices.end(), block.bottom_face.begin(), block.bottom_face.end());
                    }
                    if (IS_BIT_SET(block.faces, TOP))
                    {
                        vertices.insert(vertices.end(), block.top_face.begin(), block.top_face.end());
                    }
                    if (IS_BIT_SET(block.faces, RIGHT))
                    {
                        vertices.insert(vertices.end(), block.right_face.begin(), block.right_face.end());
                    }
                    if (IS_BIT_SET(block.faces, LEFT))
                    {
                        vertices.insert(vertices.end(), block.left_face.begin(), block.left_face.end());
                    }
                    if (IS_BIT_SET(block.faces, FRONT))
                    {
                        vertices.insert(vertices.end(), block.front_face.begin(), block.front_face.end());
                    }
                    if (IS_BIT_SET(block.faces, BACK))
                    {
                        vertices.insert(vertices.end(), block.back_face.begin(), block.back_face.end());
                    }
                }
            }
        }
    }
    vertices.shrink_to_fit();
}
