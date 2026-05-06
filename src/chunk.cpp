/**
 * @file chunk.cpp
 * @author Neil Kingdom
 * @since 14-06-2024
 * @version 1.0
 * @brief Contains operations and helper functions pertaining to Chunk objects.
 */

#include "chunk.hpp"

Chunk::Chunk() :
    location{},
    update_pending(false),
    tree_ref(),
    vertices{}
{
    this->block_heights.resize(
        KC::CHUNK_SIZE + 2,
        std::vector<uint8_t>(KC::CHUNK_SIZE + 2)
    );
    this->blocks.resize(
        KC::CHUNK_SIZE,
        std::vector<std::vector<Block>>(
            KC::CHUNK_SIZE,
            std::vector<Block>(KC::CHUNK_SIZE, Block())
        )
    );
}

Chunk::Chunk(const Vec3_t location) :
    location(location),
    update_pending(false),
    tree_ref(),
    vertices{}
{
    this->block_heights.resize(
        KC::CHUNK_SIZE + 2,
        std::vector<uint8_t>(KC::CHUNK_SIZE + 2)
    );
    this->blocks.resize(
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
    return this->location.x == chunk.location.x
        && this->location.y == chunk.location.y
        && this->location.z == chunk.location.z;
}

/**
 * @brief Squashes the block vertices into one unified mesh.
 * @since 13-02-2025
 */
void Chunk::update_mesh()
{
    this->update_pending = true;
    this->vertices.clear();

    for (size_t z = 0; z < KC::CHUNK_SIZE; ++z)
    {
        for (size_t y = 0; y < KC::CHUNK_SIZE; ++y)
        {
            for (size_t x = 0; x < KC::CHUNK_SIZE; ++x)
            {
                Block &block = blocks[z][y][x];
                if (block.type == BlockType::AIR || block.faces == 0)
                {
                    continue;
                }

                if (IS_BIT_SET(block.faces, BlockFace::BOTTOM))
                {
                    this->vertices.insert(
                        this->vertices.end(),
                        block.bottom_face.begin(),
                        block.bottom_face.end()
                    );
                }
                if (IS_BIT_SET(block.faces, BlockFace::TOP))
                {
                    this->vertices.insert(
                        this->vertices.end(),
                        block.top_face.begin(),
                        block.top_face.end()
                    );
                }
                if (IS_BIT_SET(block.faces, BlockFace::RIGHT))
                {
                    this->vertices.insert(
                        this->vertices.end(),
                        block.right_face.begin(),
                        block.right_face.end()
                    );
                }
                if (IS_BIT_SET(block.faces, BlockFace::LEFT))
                {
                    this->vertices.insert(
                        this->vertices.end(),
                        block.left_face.begin(),
                        block.left_face.end()
                    );
                }
                if (IS_BIT_SET(block.faces, BlockFace::FRONT))
                {
                    this->vertices.insert(
                        this->vertices.end(),
                        block.front_face.begin(),
                        block.front_face.end()
                    );
                }
                if (IS_BIT_SET(block.faces, BlockFace::BACK))
                {
                    this->vertices.insert(
                        this->vertices.end(),
                        block.back_face.begin(),
                        block.back_face.end()
                    );
                }
            }
        }
    }
}
