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
 * @param[in] block_location The location at which to place the block, relative to __chunk__
 * @param[in] overwrite Specifies whether to overwrite a block if one exists at the location specified by __location__
 * @returns True if the block was successfully added, otherwise returns false
 */
Result ChunkManager::add_block(
    std::shared_ptr<Chunk> &chunk,
    const BlockType type,
    const vec3 block_location,
    const bool overwrite
) const
{
    BlockFactory &block_factory = BlockFactory::get_instance();

    if (block_location[0] < 0 || block_location[0] >= KC::CHUNK_SIZE ||
        block_location[1] < 0 || block_location[1] >= KC::CHUNK_SIZE ||
        block_location[2] < 0 || block_location[2] >= KC::CHUNK_SIZE)
    {
        return Result::OOB;
    }

    Block &block = chunk->blocks[block_location[2]][block_location[1]][block_location[0]];
    if (!overwrite && block.type != BlockType::AIR)
    {
        return Result::FAILURE;
    }

    vec3 world_location = {
         (chunk->location[0] * KC::CHUNK_SIZE) + block_location[0],
         (chunk->location[1] * KC::CHUNK_SIZE) + block_location[1],
         (chunk->location[2] * KC::CHUNK_SIZE) + block_location[2]
    };
    block = block_factory.make_block(type, ALL, world_location);

    // Remove faces from block and its neighbors
    if (block.type != BlockType::LEAVES)
    {
        if (block_location[0] > 0 &&
            chunk->blocks[block_location[2]][block_location[1]][block_location[0] - 1].type != BlockType::AIR &&
            chunk->blocks[block_location[2]][block_location[1]][block_location[0] - 1].type != BlockType::LEAVES)
        {
            UNSET_BIT(chunk->blocks[block_location[2]][block_location[1]][block_location[0] - 1].faces, BACK);
            UNSET_BIT(block.faces, FRONT);
        }
        if (block_location[0] < (KC::CHUNK_SIZE - 1) &&
            chunk->blocks[block_location[2]][block_location[1]][block_location[0] + 1].type != BlockType::AIR &&
            chunk->blocks[block_location[2]][block_location[1]][block_location[0] + 1].type != BlockType::LEAVES)
        {
            UNSET_BIT(chunk->blocks[block_location[2]][block_location[1]][block_location[0] + 1].faces, FRONT);
            UNSET_BIT(block.faces, BACK);
        }
        if (block_location[1] > 0 &&
            chunk->blocks[block_location[2]][block_location[1] - 1][block_location[0]].type != BlockType::AIR &&
            chunk->blocks[block_location[2]][block_location[1] - 1][block_location[0]].type != BlockType::LEAVES)
        {
            UNSET_BIT(chunk->blocks[block_location[2]][block_location[1] - 1][block_location[0]].faces, RIGHT);
            UNSET_BIT(block.faces, LEFT);
        }
        if (block_location[1] < (KC::CHUNK_SIZE - 1) &&
            chunk->blocks[block_location[2]][block_location[1] + 1][block_location[0]].type != BlockType::AIR &&
            chunk->blocks[block_location[2]][block_location[1] + 1][block_location[0]].type != BlockType::LEAVES)
        {
            UNSET_BIT(chunk->blocks[block_location[2]][block_location[1] + 1][block_location[0]].faces, LEFT);
            UNSET_BIT(block.faces, RIGHT);
        }
        if (block_location[2] > 0 &&
            chunk->blocks[block_location[2] - 1][block_location[1]][block_location[0]].type != BlockType::AIR &&
            chunk->blocks[block_location[2] - 1][block_location[1]][block_location[0]].type != BlockType::LEAVES)
        {
            UNSET_BIT(chunk->blocks[block_location[2] - 1][block_location[1]][block_location[0]].faces, TOP);
            UNSET_BIT(block.faces, BOTTOM);
        }
        if (block_location[2] < (KC::CHUNK_SIZE - 1) &&
            chunk->blocks[block_location[2] + 1][block_location[1]][block_location[0]].type != BlockType::AIR &&
            chunk->blocks[block_location[2] + 1][block_location[1]][block_location[0]].type != BlockType::LEAVES)
        {
            UNSET_BIT(chunk->blocks[block_location[2] + 1][block_location[1]][block_location[0]].faces, BOTTOM);
            UNSET_BIT(block.faces, TOP);
        }
    }

    return Result::SUCCESS;
}

/**
 * @brief Removes a block at the location specified by __location__ relative to __chunk__'s location.
 * @since 13-02-2025
 * @param[in/out] chunk The chunk who's block is being removed
 * @param[in] block_location The location relative to __chunk__'s location where the block will be removed
 * @returns True if the block was successfully removed, otherwise false
 */
Result ChunkManager::remove_block(std::shared_ptr<Chunk> &chunk, const vec3 block_location) const
{
    Block &block = chunk->blocks[block_location[2]][block_location[1]][block_location[0]];
    block = Block();

    // TODO: Regenerate neighboring block faces

    return Result::SUCCESS;
}

/**
 * @brief Plants a tree at the location specified by __root_location__ (+1 unit in the z direction).
 * @since 13-02-2025
 * @param[in/out] chunk The chunk in which the tree will be planted
 * @param[in] root_location The location relative to __chunk__'s location at which the tree will be planted
 */
ChunkMap ChunkManager::plant_tree(std::shared_ptr<Chunk> &chunk, const vec3 root_location)
{
    auto deferred_list = ChunkMap{};

    // Trunk
    for (int i = 1; i <= 6; ++i)
    {
        vec3 block_location = { root_location[0], root_location[1], root_location[2] + i };
        if (add_block(chunk, BlockType::WOOD, block_location, true) == Result::OOB)
        {
            auto deferred = add_block_relative(chunk, BlockType::WOOD, block_location);
            deferred_list.insert(deferred);
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

            vec3 block_location1 = { root_location[0] + x, root_location[1] + y, root_location[2] + 4 };
            if (add_block(chunk, BlockType::LEAVES, block_location1, true) == Result::OOB)
            {
                auto deferred = add_block_relative(chunk, BlockType::LEAVES, block_location1);
                deferred_list.insert(deferred);
            }
            vec3 block_location2 = { root_location[0] + x, root_location[1] + y, root_location[2] + 5 };
            if (add_block(chunk, BlockType::LEAVES, block_location2, true) == Result::OOB)
            {
                auto deferred = add_block_relative(chunk, BlockType::LEAVES, block_location2);
                deferred_list.insert(deferred);
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

            vec3 block_location = { root_location[0] + x, root_location[1] + y, root_location[2] + 6 };
            if (add_block(chunk, BlockType::LEAVES, block_location, true) == Result::OOB)
            {
                auto deferred = add_block_relative(chunk, BlockType::LEAVES, block_location);
                deferred_list.insert(deferred);
            }
        }
    }

    // Leaves (second 3x3 layer is plus-shaped)
    vec3 block_location1 = { root_location[0] + 0, root_location[1] + 0, root_location[2] + 7 };
    if (add_block(chunk, BlockType::LEAVES, block_location1, true) == Result::OOB)
    {
        auto deferred = add_block_relative(chunk, BlockType::LEAVES, block_location1);
        deferred_list.insert(deferred);
    }
    vec3 block_location2 = { root_location[0] + 1, root_location[1] + 0, root_location[2] + 7 };
    if (add_block(chunk, BlockType::LEAVES, block_location2, true) == Result::OOB)
    {
        auto deferred = add_block_relative(chunk, BlockType::LEAVES, block_location2);
        deferred_list.insert(deferred);
    }
    vec3 block_location3 = { root_location[0] - 1, root_location[1] + 0, root_location[2] + 7 };
    if (add_block(chunk, BlockType::LEAVES, block_location3, true) == Result::OOB)
    {
        auto deferred = add_block_relative(chunk, BlockType::LEAVES, block_location3);
        deferred_list.insert(deferred);
    }
    vec3 block_location4 = { root_location[0] + 0, root_location[1] + 1, root_location[2] + 7 };
    if (add_block(chunk, BlockType::LEAVES, block_location4, true) == Result::OOB)
    {
        auto deferred = add_block_relative(chunk, BlockType::LEAVES, block_location4);
        deferred_list.insert(deferred);
    }
    vec3 block_location5 = { root_location[0] + 0, root_location[1] - 1, root_location[2] + 7 };
    if (add_block(chunk, BlockType::LEAVES, block_location5, true) == Result::OOB)
    {
        auto deferred = add_block_relative(chunk, BlockType::LEAVES, block_location5);
        deferred_list.insert(deferred);
    }

    return deferred_list;
}

/**
 * @brief Plants trees within the visible frustum after terrain has been generated.
 *
 * TODO: Params
 */
ChunkMap ChunkManager::plant_trees(std::shared_ptr<Chunk> &chunk)
{
    auto deferred_list = ChunkMap{};
    const unsigned rand_threshold = 576;

    for (ssize_t y = 0, _y = 1; y < KC::CHUNK_SIZE; ++y, ++_y)
    {
        for (ssize_t x = 0, _x = 1; x < KC::CHUNK_SIZE; ++x, ++_x)
        {
            ssize_t z_offset = chunk->block_heights[_y][_x] / KC::CHUNK_SIZE;
            if (z_offset != chunk->location[2])
            {
                continue;
            }

            ssize_t z = chunk->block_heights[_y][_x] % KC::CHUNK_SIZE;
            vec3 root_location = { (float)x, (float)y, (float)z };
            if (fnv1a_hash(chunk->location, root_location) % rand_threshold == 0)
            {
                auto deferred = plant_tree(chunk, root_location);
                deferred_list.insert(deferred.begin(), deferred.end());
            }
        }
    }

    return deferred_list;
}

void ChunkManager::update_mesh()
{
    // Check if any chunks have been modified
    bool chunk_update_pending = std::any_of(
        GCL.begin(),
        GCL.end(),
        [&](const auto &kv_pair)
        {
            return kv_pair.second->update_pending;
        }
    );

    if (chunk_update_pending)
    {
        terrain_mesh.vertices.clear();
        for (auto &chunk : GCL.values())
        {
            chunk->update_pending = false;
            if (!chunk->vertices.empty())
            {
                terrain_mesh.vertices.insert(
                    terrain_mesh.vertices.end(),
                    chunk->vertices.begin(),
                    chunk->vertices.end()
                );
            }
        }

        // Bind mesh to VBO
        glBindBuffer(GL_ARRAY_BUFFER, terrain_mesh.vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            terrain_mesh.vertices.size() * sizeof(VPosTex),
            terrain_mesh.vertices.data(),
            GL_STATIC_DRAW
        );
        glBindBuffer(0, terrain_mesh.vbo);
    }
}

/**
 * @brief Adds a block relative to the current chunk.
 * This function is used when we calculate a block position that is outside of the current chunk bound and
 * want to add it to the correct parent chunk.
 * @warning If the parent chunk does not exist, it will be created, though the mesh will not be updated.
 * @param[in] block_factory Reference to a BlockFactory object
 * @param[in] chunk_location Location of the current chunk as a vec3
 * @param[in] block_location Location of the block to be placed, relative to the current chunk, as a vec3
 * @param[in] type The type of block that will be created
 */
std::shared_ptr<Chunk> ChunkManager::add_block_relative(
    std::shared_ptr<Chunk> &chunk,
    const BlockType type,
    const vec3 block_location
)
{
    ChunkFactory &chunk_factory = ChunkFactory::get_instance();

    vec3 actual_chunk_location{};
    vec3 actual_block_location{};

    actual_chunk_location[0] = std::floorf(((chunk->location[0] * KC::CHUNK_SIZE) + block_location[0]) / KC::CHUNK_SIZE);
    actual_chunk_location[1] = std::floorf(((chunk->location[1] * KC::CHUNK_SIZE) + block_location[1]) / KC::CHUNK_SIZE);
    actual_chunk_location[2] = std::floorf(((chunk->location[2] * KC::CHUNK_SIZE) + block_location[2]) / KC::CHUNK_SIZE);

    actual_block_location[0] = (((int)block_location[0] % KC::CHUNK_SIZE) + KC::CHUNK_SIZE) % KC::CHUNK_SIZE;
    actual_block_location[1] = (((int)block_location[1] % KC::CHUNK_SIZE) + KC::CHUNK_SIZE) % KC::CHUNK_SIZE;
    actual_block_location[2] = (((int)block_location[2] % KC::CHUNK_SIZE) + KC::CHUNK_SIZE) % KC::CHUNK_SIZE;

    auto needle = GCL.find(actual_chunk_location);
    if (needle != nullptr)
    {
        // Use existing chunk
        add_block(needle, type, actual_block_location, false);
        return needle;
    }
    else
    {
        // Create new chunk
        auto new_chunk = chunk_factory.make_chunk(actual_chunk_location);
        add_block(new_chunk, type, actual_block_location, false);
        new_chunk->tree_ref = chunk;
        GCL.insert(new_chunk);
        return new_chunk;
    }
}
