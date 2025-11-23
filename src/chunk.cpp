/**
 * @file chunk.cpp
 * @author Neil Kingdom
 * @since 14-06-2024
 * @version 1.0
 * @brief Contains operations and helper functions pertaining to Chunk objects.
 */

#include "chunk.hpp"

Chunk::Chunk() :
    update_pending(false), vertices{}
{
    block_heights.resize(
        KC::CHUNK_SIZE + 2,
        std::vector<uint8_t>(KC::CHUNK_SIZE + 2)
    );
    blocks.resize(
        KC::CHUNK_SIZE,
        std::vector<std::vector<Block>>(
            KC::CHUNK_SIZE,
            std::vector<Block>(KC::CHUNK_SIZE, Block())
        )
    );
}

Chunk::Chunk(const vec3 location) :
    update_pending(false), vertices{}
{
    std::memcpy(this->location, location, sizeof(vec3));
    block_heights.resize(
        KC::CHUNK_SIZE + 2,
        std::vector<uint8_t>(KC::CHUNK_SIZE + 2)
    );
    blocks.resize(
        KC::CHUNK_SIZE,
        std::vector<std::vector<Block>>(
            KC::CHUNK_SIZE,
            std::vector<Block>(KC::CHUNK_SIZE, Block())
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
    return V3_EQ(this->location, chunk.location);
}

/**
 * @brief Squashes the block vertices into one unified mesh.
 * @since 13-02-2025
 */
void Chunk::update_mesh()
{
    update_pending = true;
    vertices.clear();

    for (ssize_t z = 0; z < KC::CHUNK_SIZE; ++z)
    {
        for (ssize_t y = 0; y < KC::CHUNK_SIZE; ++y)
        {
            for (ssize_t x = 0; x < KC::CHUNK_SIZE; ++x)
            {
                Block &block = blocks[z][y][x];
                if (block.type != BlockType::AIR && block.faces != 0)
                {
                    if (IS_BIT_SET(block.faces, BlockFace::BOTTOM))
                    {
                        vertices.insert(vertices.end(), block.bottom_face.begin(), block.bottom_face.end());
                    }
                    if (IS_BIT_SET(block.faces, BlockFace::TOP))
                    {
                        vertices.insert(vertices.end(), block.top_face.begin(), block.top_face.end());
                    }
                    if (IS_BIT_SET(block.faces, BlockFace::RIGHT))
                    {
                        vertices.insert(vertices.end(), block.right_face.begin(), block.right_face.end());
                    }
                    if (IS_BIT_SET(block.faces, BlockFace::LEFT))
                    {
                        vertices.insert(vertices.end(), block.left_face.begin(), block.left_face.end());
                    }
                    if (IS_BIT_SET(block.faces, BlockFace::FRONT))
                    {
                        vertices.insert(vertices.end(), block.front_face.begin(), block.front_face.end());
                    }
                    if (IS_BIT_SET(block.faces, BlockFace::BACK))
                    {
                        vertices.insert(vertices.end(), block.back_face.begin(), block.back_face.end());
                    }
                }
            }
        }
    }
}
