/**
 * @file chunk_manager.cpp
 * @author Neil Kingdom
 * @since 13-02-2025
 * @version 1.0
 * @brief Singleton class that manages chunks.
 * Adds functionality for retrieving chunks, adding or removing blocks, planting trees, and
 * will cache chunks that have been modified by the player.
 */

#include "chunk_manager.hpp"

/**
 * @brief Entry point for accessing the singleton's instance.
 * @since 13-02-2025
 * @returns A reference to the single ChunkManager instance;
 */
ChunkManager &ChunkManager::get_instance()
{
    static ChunkManager chunk_mgr;
    return chunk_mgr;
}

/**
 * @brief Adds a block at the specified __location__ relative to __chunk__'s position.
 * @since 13-02-2025
 * @param[in/out] chunk The chunk that the block will be added to
 * @param[in] type The type of block that we want to add to __chunk__
 * @param[in] location The location at which to place the block, relative to __chunk__
 * @param[in] overwrite Specifies whether to overwrite a block if one exists at the location specified by __location__
 * @returns True if the block was successfully added, otherwise returns false
 */
bool ChunkManager::add_block(
    std::shared_ptr<Chunk> &chunk,
    const BlockType type,
    const vec3 location,
    const bool overwrite
)
{
    BlockFactory block_factory;
    Settings &settings = Settings::get_instance();
    ssize_t chunk_size = settings.chunk_size;

    if (location[0] < 0 || location[0] >= chunk_size ||
        location[1] < 0 || location[1] >= chunk_size ||
        location[2] < 0 || location[2] >= chunk_size)
    {
        return false;
    }

    if (!overwrite && chunk->blocks[location[2]][location[1]][location[0]].type != BlockType::AIR)
    {
        return false;
    }

    Block &block = chunk->blocks[location[2]][location[1]][location[0]];
    if (block.type == BlockType::AIR)
    {
        vec3 real_location = {
            -(chunk->location[0] * chunk_size) + location[0],
             (chunk->location[1] * chunk_size) + location[1],
             (chunk->location[2] * chunk_size) + location[2]
        };
        block = block_factory.make_block(type, real_location, ALL);
    }

    block.faces = ALL;
    block.type = type;

    // Remove block faces

    if (block.type != BlockType::LEAVES)
    {
        if (
            location[0] > 0 &&
            chunk->blocks[location[2]][location[1]][location[0] - 1].type != BlockType::AIR &&
            chunk->blocks[location[2]][location[1]][location[0] - 1].type != BlockType::LEAVES
        )
        {
            UNSET_BIT(chunk->blocks[location[2]][location[1]][location[0] - 1].faces, BACK);
            UNSET_BIT(block.faces, FRONT);
        }
        if (
            location[0] < (chunk_size - 1) &&
            chunk->blocks[location[2]][location[1]][location[0] + 1].type != BlockType::AIR &&
            chunk->blocks[location[2]][location[1]][location[0] + 1].type != BlockType::LEAVES
        )
        {
            UNSET_BIT(chunk->blocks[location[2]][location[1]][location[0] + 1].faces, FRONT);
            UNSET_BIT(block.faces, BACK);
        }
        if (location[1] > 0 &&
            chunk->blocks[location[2]][location[1] - 1][location[0]].type != BlockType::AIR &&
            chunk->blocks[location[2]][location[1] - 1][location[0]].type != BlockType::LEAVES
        )
        {
            UNSET_BIT(chunk->blocks[location[2]][location[1] - 1][location[0]].faces, RIGHT);
            UNSET_BIT(block.faces, LEFT);
        }
        if (location[1] < (chunk_size - 1) &&
            chunk->blocks[location[2]][location[1] + 1][location[0]].type != BlockType::AIR &&
            chunk->blocks[location[2]][location[1] + 1][location[0]].type != BlockType::LEAVES
        )
        {
            UNSET_BIT(chunk->blocks[location[2]][location[1] + 1][location[0]].faces, LEFT);
            UNSET_BIT(block.faces, RIGHT);
        }
        if (location[2] > 0 &&
            chunk->blocks[location[2] - 1][location[1]][location[0]].type != BlockType::AIR &&
            chunk->blocks[location[2] - 1][location[1]][location[0]].type != BlockType::LEAVES
        )
        {
            UNSET_BIT(chunk->blocks[location[2] - 1][location[1]][location[0]].faces, TOP);
            UNSET_BIT(block.faces, BOTTOM);
        }
        if (location[2] < (chunk_size - 1) &&
            chunk->blocks[location[2] + 1][location[1]][location[0]].type != BlockType::AIR &&
            chunk->blocks[location[2] + 1][location[1]][location[0]].type != BlockType::LEAVES
        )
        {
            UNSET_BIT(chunk->blocks[location[2] + 1][location[1]][location[0]].faces, BOTTOM);
            UNSET_BIT(block.faces, TOP);
        }
    }

    // TODO: Add chunk to cache if not already present

    return true;
}

/**
 * @brief Removes a block at the location specified by __location__ relative to __chunk__'s location.
 * @since 13-02-2025
 * @param[in/out] chunk The chunk who's block is being removed
 * @param[in] location The location relative to __chunk__'s location where the block will be removed
 * @returns True if the block was successfully removed, otherwise false
 */
bool ChunkManager::remove_block(std::shared_ptr<Chunk> &chunk, const vec3 location)
{
    chunk->blocks[location[2]][location[1]][location[0]].type = BlockType::AIR;

    // TODO: Regenerate neighboring block faces

    return false;
}

/**
 * @brief Plants a tree at the location specified by __location__ (+1 unit in the z direction).
 * @since 13-02-2025
 * @param[in/out] chunk The chunk in which the tree will be planted
 * @param[in] location The location relative to __chunk__'s location at which the tree will be planted
 */
void ChunkManager::plant_tree(std::shared_ptr<Chunk> &chunk, const vec3 location)
{
    // Trunk
    for (int i = 1; i <= 6; ++i)
    {
        add_block(chunk, BlockType::WOOD, vec3{ location[0], location[1], location[2] + i }, true);
    }

    // Leaves (two 5x5 layers)
    for (int y = -2; y <= 2; ++y)
    {
        for (int x = -2; x <= 2; ++x)
        {
            if (x == 0 && y == 0)
            {
                continue;
            }
            add_block(chunk, BlockType::LEAVES, vec3{ location[0] + x, location[1] + y, location[2] + 4 }, true);
            add_block(chunk, BlockType::LEAVES, vec3{ location[0] + x, location[1] + y, location[2] + 5 }, true);
        }
    }

    // Leaves (first 3x3 layer)
    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            if (x == 0 && y == 0)
            {
                continue;
            }
            add_block(chunk, BlockType::LEAVES, vec3{ location[0] + x, location[1] + y, location[2] + 6 }, true);
        }
    }

    // Leaves (second 3x3 layer: x-shaped)
    add_block(chunk, BlockType::LEAVES, vec3{ location[0] + 0, location[1] + 0, location[2] + 7 }, true);
    add_block(chunk, BlockType::LEAVES, vec3{ location[0] + 1, location[1] + 0, location[2] + 7 }, true);
    add_block(chunk, BlockType::LEAVES, vec3{ location[0] - 1, location[1] + 0, location[2] + 7 }, true);
    add_block(chunk, BlockType::LEAVES, vec3{ location[0] + 0, location[1] + 1, location[2] + 7 }, true);
    add_block(chunk, BlockType::LEAVES, vec3{ location[0] + 0, location[1] - 1, location[2] + 7 }, true);
}

/**
 * @brief Optionally returns the chunk at the location specified by __location__ if it exists.
 * @since 13-02-2025
 * @param location A vec3 that specifies the location of the chunk to return; measured in units of blocks
 * @returns The chunk at location __location__ if it exists, otherwise returns std::nullopt
 */
std::optional<std::shared_ptr<Chunk>> ChunkManager::get_chunk_at_block_offset(const vec3 location) const
{
    Settings &settings = Settings::get_instance();
    ssize_t chunk_size = settings.chunk_size;

    vec3 chunk_offset = {
        (float)((ssize_t)location[0] % chunk_size),
        (float)((ssize_t)location[1] % chunk_size),
        (float)((ssize_t)location[2] % chunk_size),
    };

    return get_chunk_at_chunk_offset(chunk_offset);
}

/**
 * @brief Optionally returns the chunk at the location specified by __location__ if it exists.
 * @since 13-02-2025
 * @param location A vec3 that specifies the location of the chunk to return; measured in units of chunks
 * @returns The chunk at location __location__ if it exists, otherwise returns std::nullopt
 */
std::optional<std::shared_ptr<Chunk>> ChunkManager::get_chunk_at_chunk_offset(const vec3 location) const
{
    Chunk needle;
    std::memcpy(needle.location, location, sizeof(vec3));

    auto found = std::find_if(
        chunks.begin(),
        chunks.end(),
        [&](const std::shared_ptr<Chunk> &chunk)
        {
            return needle == *chunk;
        }
    );

    return (found != chunks.end()) ? std::make_optional(*found) : std::nullopt;
}
