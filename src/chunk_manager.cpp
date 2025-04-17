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

ChunkManager::ChunkManager()
{
    glGenVertexArrays(1, &terrain_mesh.vao);
    glBindVertexArray(terrain_mesh.vao);

    glGenBuffers(1, &terrain_mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, terrain_mesh.vbo);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // TODO: Color attribute

    // Texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

ChunkManager::~ChunkManager()
{
    if (glIsBuffer(terrain_mesh.vbo))
    {
        glDeleteBuffers(1, &terrain_mesh.vbo);
    }
    if (glIsVertexArray(terrain_mesh.vao))
    {
        glDeleteVertexArrays(1, &terrain_mesh.vao);
    }
}

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
Result ChunkManager::add_block(
    const BlockFactory &block_factory,
    std::shared_ptr<Chunk> &chunk,
    const BlockType type,
    const vec3 location,
    const bool overwrite
)
{
    Settings &settings = Settings::get_instance();
    ssize_t chunk_size = settings.chunk_size;

    if (location[0] < 0 || location[0] >= chunk_size ||
        location[1] < 0 || location[1] >= chunk_size ||
        location[2] < 0 || location[2] >= chunk_size)
    {
        return Result::OOB;
    }

    if (!overwrite && chunk->blocks[location[2]][location[1]][location[0]].type != BlockType::AIR)
    {
        return Result::FAILURE;
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

    return Result::SUCCESS;
}

/**
 * @brief Removes a block at the location specified by __location__ relative to __chunk__'s location.
 * @since 13-02-2025
 * @param[in/out] chunk The chunk who's block is being removed
 * @param[in] location The location relative to __chunk__'s location where the block will be removed
 * @returns True if the block was successfully removed, otherwise false
 */
Result ChunkManager::remove_block(std::shared_ptr<Chunk> &chunk, const vec3 location)
{
    chunk->blocks[location[2]][location[1]][location[0]].type = BlockType::AIR;

    // TODO: Regenerate neighboring block faces

    return Result::FAILURE;
}

/**
 * @brief Optionally returns the chunk at the location specified by __location__ if it exists.
 * @since 13-02-2025
 * @param location A vec3 that specifies the location of the chunk to return; measured in units of blocks
 * @returns The chunk at location __location__ if it exists, otherwise returns std::nullopt
 */
std::optional<std::shared_ptr<Chunk>> ChunkManager::get_chunk(const Block &block) const
{
    return std::nullopt;
}

/**
 * @brief Optionally returns the chunk at the location specified by __location__ if it exists.
 * @since 13-02-2025
 * @param location A vec3 that specifies the location of the chunk to return; measured in units of chunks
 * @returns The chunk at location __location__ if it exists, otherwise returns std::nullopt
 */
std::optional<std::shared_ptr<Chunk>> ChunkManager::get_chunk(const Chunk &chunk) const
{
    auto found = std::find_if(
        chunks.begin(),
        chunks.end(),
        [&](const std::shared_ptr<Chunk> &item)
        {
            return chunk == *item;
        }
    );

    return (found != chunks.end()) ? std::make_optional(*found) : std::nullopt;
}

/**
 * @brief Plants a tree at the location specified by __location__ (+1 unit in the z direction).
 * @since 13-02-2025
 * @param[in/out] chunk The chunk in which the tree will be planted
 * @param[in] location The location relative to __chunk__'s location at which the tree will be planted
 */
void ChunkManager::plant_tree(const BlockFactory &block_factory, std::shared_ptr<Chunk> &chunk, const vec3 location)
{
    // Trunk
    for (int i = 1; i <= 6; ++i)
    {
        vec3 block_location = { location[0], location[1], location[2] + i };
        if (add_block(block_factory, chunk, BlockType::WOOD, block_location, true) == Result::OOB)
        {
            add_block_relative_to_current(block_factory, chunk->location, block_location);
        }
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

            vec3 block_location1 = { location[0] + x, location[1] + y, location[2] + 4 };
            if (add_block(block_factory, chunk, BlockType::LEAVES, block_location1, true) == Result::OOB)
            {
                add_block_relative_to_current(block_factory, chunk->location, block_location1);
            }
            vec3 block_location2 = { location[0] + x, location[1] + y, location[2] + 5 };
            if (add_block(block_factory, chunk, BlockType::LEAVES, block_location2, true) == Result::OOB)
            {
                add_block_relative_to_current(block_factory, chunk->location, block_location2);
            }
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

            vec3 block_location = { location[0] + x, location[1] + y, location[2] + 6 };
            if (add_block(block_factory, chunk, BlockType::LEAVES, block_location, true) == Result::OOB)
            {
                add_block_relative_to_current(block_factory, chunk->location, block_location);
            }
        }
    }

    // Leaves (second 3x3 layer: +shaped)
    vec3 block_location1 = { location[0] + 0, location[1] + 0, location[2] + 7 };
    if (add_block(block_factory, chunk, BlockType::LEAVES, block_location1, true) == Result::OOB)
    {
        add_block_relative_to_current(block_factory, chunk->location, block_location1);
    }
    vec3 block_location2 = { location[0] + 1, location[1] + 0, location[2] + 7 };
    if (add_block(block_factory, chunk, BlockType::LEAVES, block_location2, true) == Result::OOB)
    {
        add_block_relative_to_current(block_factory, chunk->location, block_location2);
    }
    vec3 block_location3 = { location[0] - 1, location[1] + 0, location[2] + 7 };
    if (add_block(block_factory, chunk, BlockType::LEAVES, block_location3, true) == Result::OOB)
    {
        add_block_relative_to_current(block_factory, chunk->location, block_location3);
    }
    vec3 block_location4 = { location[0] + 0, location[1] + 1, location[2] + 7 };
    if (add_block(block_factory, chunk, BlockType::LEAVES, block_location4, true) == Result::OOB)
    {
        add_block_relative_to_current(block_factory, chunk->location, block_location4);
    }
    vec3 block_location5 = { location[0] + 0, location[1] - 1, location[2] + 7 };
    if (add_block(block_factory, chunk, BlockType::LEAVES, block_location5, true) == Result::OOB)
    {
        add_block_relative_to_current(block_factory, chunk->location, block_location5);
    }
}

void ChunkManager::update_mesh()
{
    if (std::any_of(chunks.begin(), chunks.end(),
        [&](std::shared_ptr<Chunk> &chunk)
        {
            return chunk->updated;
        })
    )
    {
        terrain_mesh.vertices.clear();
        for (auto chunk : chunks)
        {
            terrain_mesh.vertices.insert(terrain_mesh.vertices.end(), chunk->vertices.begin(), chunk->vertices.end());
        }

        glBindBuffer(GL_ARRAY_BUFFER, terrain_mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, terrain_mesh.vertices.size() * sizeof(BlockVertex), terrain_mesh.vertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(0, terrain_mesh.vbo);
    }
}

void ChunkManager::add_block_relative_to_current(
    const BlockFactory &block_factory,
    const vec3 &chunk_location,
    const vec3 &block_location
)
{
    vec3 actual_chunk_location{};
    vec3 actual_block_location{};

    actual_chunk_location[0] = std::floorf(((chunk_location[0] * 16) + block_location[0]) / 16.0f);
    actual_chunk_location[1] = std::floorf(((chunk_location[1] * 16) + block_location[1]) / 16.0f);
    actual_chunk_location[2] = std::floorf(((chunk_location[2] * 16) + block_location[2]) / 16.0f);

    actual_block_location[0] = ((int)block_location[0] % 16 + 16) % 16;
    actual_block_location[1] = ((int)block_location[1] % 16 + 16) % 16;
    actual_block_location[2] = ((int)block_location[2] % 16 + 16) % 16;

    auto chunk = get_chunk(Chunk(actual_chunk_location));
    if (chunk == std::nullopt)
    {
        return;
    }

    add_block(block_factory, chunk.value(), BlockType::DIRT, actual_block_location, false);
}
