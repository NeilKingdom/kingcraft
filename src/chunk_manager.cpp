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
    glGenVertexArrays(1, &this->terrain_mesh.vao);
    glBindVertexArray(this->terrain_mesh.vao);

    glGenBuffers(1, &this->terrain_mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->terrain_mesh.vbo);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Color attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

ChunkManager::~ChunkManager()
{
    if (glIsBuffer(this->terrain_mesh.vbo))
    {
        glDeleteBuffers(1, &this->terrain_mesh.vbo);
    }
    if (glIsVertexArray(this->terrain_mesh.vao))
    {
        glDeleteVertexArrays(1, &this->terrain_mesh.vao);
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
    const Vec3_t block_location,
    const bool overwrite
) const
{
    BlockFactory &block_factory = BlockFactory::get_instance();

    if (block_location.x < 0 || block_location.x >= KC::CHUNK_SIZE ||
        block_location.y < 0 || block_location.y >= KC::CHUNK_SIZE ||
        block_location.z < 0 || block_location.z >= KC::CHUNK_SIZE)
    {
        return Result::OOB;
    }

    Block &block = chunk->blocks[block_location.z][block_location.y][block_location.x];
    if (!overwrite && block.type != BlockType::AIR)
    {
        return Result::FAILURE;
    }

    Vec3_t world_location = { .v = {
         (chunk->location.x * KC::CHUNK_SIZE) + block_location.x,
         (chunk->location.y * KC::CHUNK_SIZE) + block_location.y,
         (chunk->location.z * KC::CHUNK_SIZE) + block_location.z
    }};
    block = block_factory.make_block(type, ALL, world_location);

    // Remove faces from block and its neighbors
    if (block.type != BlockType::LEAVES)
    {
        if (block_location.x > 0 &&
            chunk->blocks[block_location.z][block_location.y][block_location.x - 1].type != BlockType::AIR &&
            chunk->blocks[block_location.z][block_location.y][block_location.x - 1].type != BlockType::LEAVES)
        {
            UNSET_BIT(chunk->blocks[block_location.z][block_location.y][block_location.x - 1].faces, BACK);
            UNSET_BIT(block.faces, FRONT);
        }
        if (block_location.x < (KC::CHUNK_SIZE - 1) &&
            chunk->blocks[block_location.z][block_location.y][block_location.x + 1].type != BlockType::AIR &&
            chunk->blocks[block_location.z][block_location.y][block_location.x + 1].type != BlockType::LEAVES)
        {
            UNSET_BIT(chunk->blocks[block_location.z][block_location.y][block_location.x + 1].faces, FRONT);
            UNSET_BIT(block.faces, BACK);
        }
        if (block_location.y > 0 &&
            chunk->blocks[block_location.z][block_location.y - 1][block_location.x].type != BlockType::AIR &&
            chunk->blocks[block_location.z][block_location.y - 1][block_location.x].type != BlockType::LEAVES)
        {
            UNSET_BIT(chunk->blocks[block_location.z][block_location.y - 1][block_location.x].faces, RIGHT);
            UNSET_BIT(block.faces, LEFT);
        }
        if (block_location.y < (KC::CHUNK_SIZE - 1) &&
            chunk->blocks[block_location.z][block_location.y + 1][block_location.x].type != BlockType::AIR &&
            chunk->blocks[block_location.z][block_location.y + 1][block_location.x].type != BlockType::LEAVES)
        {
            UNSET_BIT(chunk->blocks[block_location.z][block_location.y + 1][block_location.x].faces, LEFT);
            UNSET_BIT(block.faces, RIGHT);
        }
        if (block_location.z > 0 &&
            chunk->blocks[block_location.z - 1][block_location.y][block_location.x].type != BlockType::AIR &&
            chunk->blocks[block_location.z - 1][block_location.y][block_location.x].type != BlockType::LEAVES)
        {
            UNSET_BIT(chunk->blocks[block_location.z - 1][block_location.y][block_location.x].faces, TOP);
            UNSET_BIT(block.faces, BOTTOM);
        }
        if (block_location.z < (KC::CHUNK_SIZE - 1) &&
            chunk->blocks[block_location.z + 1][block_location.y][block_location.x].type != BlockType::AIR &&
            chunk->blocks[block_location.z + 1][block_location.y][block_location.x].type != BlockType::LEAVES)
        {
            UNSET_BIT(chunk->blocks[block_location.z + 1][block_location.y][block_location.x].faces, BOTTOM);
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
Result ChunkManager::remove_block(std::shared_ptr<Chunk> &chunk, const Vec3_t block_location) const
{
    Block &block = chunk->blocks[block_location.z][block_location.y][block_location.x];
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
ChunkMap ChunkManager::plant_tree(std::shared_ptr<Chunk> &chunk, const Vec3_t root_location)
{
    auto deferred_list = ChunkMap{};

    // Trunk
    for (int i = 1; i <= 6; ++i)
    {
        Vec3_t block_location = { .v = { root_location.x, root_location.y, root_location.z + i }};
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

            Vec3_t block_location1 = { .v = { root_location.x + x, root_location.y + y, root_location.z + 4 }};
            if (add_block(chunk, BlockType::LEAVES, block_location1, true) == Result::OOB)
            {
                auto deferred = add_block_relative(chunk, BlockType::LEAVES, block_location1);
                deferred_list.insert(deferred);
            }
            Vec3_t block_location2 = { .v = { root_location.x + x, root_location.y + y, root_location.z + 5 }};
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

            Vec3_t block_location = { .v = { root_location.x + x, root_location.y + y, root_location.z + 6 }};
            if (add_block(chunk, BlockType::LEAVES, block_location, true) == Result::OOB)
            {
                auto deferred = add_block_relative(chunk, BlockType::LEAVES, block_location);
                deferred_list.insert(deferred);
            }
        }
    }

    // Leaves (second 3x3 layer is plus-shaped)
    Vec3_t block_location1 = { .v = { root_location.x, root_location.y, root_location.z + 7 }};
    if (add_block(chunk, BlockType::LEAVES, block_location1, true) == Result::OOB)
    {
        auto deferred = add_block_relative(chunk, BlockType::LEAVES, block_location1);
        deferred_list.insert(deferred);
    }
    Vec3_t block_location2 = { .v = { root_location.x + 1, root_location.y, root_location.z + 7 }};
    if (add_block(chunk, BlockType::LEAVES, block_location2, true) == Result::OOB)
    {
        auto deferred = add_block_relative(chunk, BlockType::LEAVES, block_location2);
        deferred_list.insert(deferred);
    }
    Vec3_t block_location3 = { .v = { root_location.x - 1, root_location.y, root_location.z + 7 }};
    if (add_block(chunk, BlockType::LEAVES, block_location3, true) == Result::OOB)
    {
        auto deferred = add_block_relative(chunk, BlockType::LEAVES, block_location3);
        deferred_list.insert(deferred);
    }
    Vec3_t block_location4 = { .v = { root_location.x, root_location.y + 1, root_location.z + 7 }};
    if (add_block(chunk, BlockType::LEAVES, block_location4, true) == Result::OOB)
    {
        auto deferred = add_block_relative(chunk, BlockType::LEAVES, block_location4);
        deferred_list.insert(deferred);
    }
    Vec3_t block_location5 = { .v = { root_location.x, root_location.y - 1, root_location.z + 7 }};
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
ChunkMap ChunkManager::plant_trees(std::shared_ptr<Chunk> &chunk, const float density)
{
    auto deferred_list = ChunkMap{};

    for (size_t y = 0, _y = 1; y < KC::CHUNK_SIZE; ++y, ++_y)
    {
        for (size_t x = 0, _x = 1; x < KC::CHUNK_SIZE; ++x, ++_x)
        {
            size_t z_chunk = chunk->block_heights[_y][_x] / KC::CHUNK_SIZE;
            if (z_chunk != chunk->location.z)
            {
                continue;
            }

            size_t z = chunk->block_heights[_y][_x] % KC::CHUNK_SIZE;
            Vec3_t root_location = { .v = { (float)x, (float)y, (float)z }};

            // Pseudo-random hash function determines if tree should be planted
            uint32_t hash = world_hash(chunk->location, root_location);
            float normalized = (float)hash / (float)UINT32_MAX;

            if (normalized < density)
            {
                auto deferred = plant_tree(chunk, root_location);
                deferred_list.insert(deferred.begin(), deferred.end());
            }
        }
    }

    return deferred_list;
}

void ChunkManager::bind_terrain_mesh()
{
    // Check if any chunks have been modified
    bool chunk_update_pending = std::any_of(
        this->GCL.begin(),
        this->GCL.end(),
        [&](const auto &kv_pair)
        {
            return kv_pair.second->update_pending;
        }
    );

    if (chunk_update_pending)
    {
        this->terrain_mesh.vertices.clear();
        for (auto &chunk : this->GCL.values())
        {
            chunk->update_pending = false;
            if (!chunk->vertices.empty())
            {
                this->terrain_mesh.vertices.insert(
                    this->terrain_mesh.vertices.end(),
                    chunk->vertices.begin(),
                    chunk->vertices.end()
                );
            }
        }

        // Bind mesh to VBO
        glBindBuffer(GL_ARRAY_BUFFER, this->terrain_mesh.vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            this->terrain_mesh.vertices.size() * sizeof(Vertex),
            this->terrain_mesh.vertices.data(),
            GL_STATIC_DRAW
        );
        glBindBuffer(0, this->terrain_mesh.vbo);
    }
}

void ChunkManager::get_relative_locations(
    const Vec3_t &chunk_location,
    const Vec3_t &block_location,
    Vec3_t &actual_chunk_location,
    Vec3_t &actual_block_location
) const
{
    actual_chunk_location.x = std::floorf(((chunk_location.x * KC::CHUNK_SIZE) + block_location.x) / KC::CHUNK_SIZE);
    actual_chunk_location.y = std::floorf(((chunk_location.y * KC::CHUNK_SIZE) + block_location.y) / KC::CHUNK_SIZE);
    actual_chunk_location.z = std::floorf(((chunk_location.z * KC::CHUNK_SIZE) + block_location.z) / KC::CHUNK_SIZE);

    actual_block_location.x = (((int)block_location.x % KC::CHUNK_SIZE) + KC::CHUNK_SIZE) % KC::CHUNK_SIZE;
    actual_block_location.y = (((int)block_location.y % KC::CHUNK_SIZE) + KC::CHUNK_SIZE) % KC::CHUNK_SIZE;
    actual_block_location.z = (((int)block_location.z % KC::CHUNK_SIZE) + KC::CHUNK_SIZE) % KC::CHUNK_SIZE;
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
    const Vec3_t block_location
)
{
    ChunkFactory &chunk_factory = ChunkFactory::get_instance();

    Vec3_t actual_chunk_location{};
    Vec3_t actual_block_location{};
    get_relative_locations(
        chunk->location,
        block_location,
        actual_chunk_location,
        actual_block_location
    );

    auto needle = this->GCL.find(actual_chunk_location);
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
        this->GCL.insert(new_chunk);
        return new_chunk;
    }
}
