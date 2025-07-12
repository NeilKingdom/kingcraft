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
    const BlockFactory &block_factory,
    const BlockType type,
    const vec3 block_location,
    const bool overwrite
) const
{
    Settings &settings = Settings::get_instance();
    ssize_t chunk_size = settings.chunk_size;

    if (block_location[0] < 0 || block_location[0] >= chunk_size ||
        block_location[1] < 0 || block_location[1] >= chunk_size ||
        block_location[2] < 0 || block_location[2] >= chunk_size)
    {
        return Result::OOB;
    }

    if (!overwrite && chunk->blocks[block_location[2]][block_location[1]][block_location[0]].type != BlockType::AIR)
    {
        return Result::FAILURE;
    }

    vec3 world_location = {
        -(chunk->location[0] * chunk_size) + block_location[0],
         (chunk->location[1] * chunk_size) + block_location[1],
         (chunk->location[2] * chunk_size) + block_location[2]
    };
    Block &block = chunk->blocks[block_location[2]][block_location[1]][block_location[0]];
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
        if (block_location[0] < (chunk_size - 1) &&
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
        if (block_location[1] < (chunk_size - 1) &&
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
        if (block_location[2] < (chunk_size - 1) &&
            chunk->blocks[block_location[2] + 1][block_location[1]][block_location[0]].type != BlockType::AIR &&
            chunk->blocks[block_location[2] + 1][block_location[1]][block_location[0]].type != BlockType::LEAVES)
        {
            UNSET_BIT(chunk->blocks[block_location[2] + 1][block_location[1]][block_location[0]].faces, BOTTOM);
            UNSET_BIT(block.faces, TOP);
        }
    }

    //chunk->update_mesh();

    // TODO: Add chunk to cache if not already present

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
    chunk->blocks[block_location[2]][block_location[1]][block_location[0]].type = BlockType::AIR;

    // TODO: Regenerate neighboring block faces

    return Result::SUCCESS;
}

/**
 * @brief Optionally returns the chunk at the location specified by __location__ if it exists.
 * @since 13-02-2025
 * @param location A vec3 that specifies the location of the chunk to return; measured in units of blocks
 * @returns The chunk at location __location__ if it exists, otherwise returns std::nullopt
 */
//std::optional<std::shared_ptr<Chunk>> ChunkManager::get_chunk(const Block &block) const
//{
//    return std::nullopt;
//}

/**
 * @brief Optionally returns the chunk at the location specified by __location__ if it exists.
 * @since 13-02-2025
 * @param location A vec3 that specifies the location of the chunk to return; measured in units of chunks
 * @returns The chunk at location __location__ if it exists, otherwise returns std::nullopt
 */
//std::optional<std::shared_ptr<Chunk>> ChunkManager::get_chunk(const std::shared_ptr<Chunk> &chunk) const
//{
//    auto it = chunks.find(chunk);
//    return (it == chunks.end()) ? std::nullopt : std::make_optional(chunk);
//}

/**
 * @brief Plants a tree at the location specified by __root_location__ (+1 unit in the z direction).
 * @since 13-02-2025
 * @param[in/out] chunk The chunk in which the tree will be planted
 * @param[in] root_location The location relative to __chunk__'s location at which the tree will be planted
 */
std::unordered_set<std::shared_ptr<Chunk>, ChunkHash, ChunkEqual>
ChunkManager::plant_tree(
    std::shared_ptr<Chunk> &chunk,
    const BlockFactory &block_factory,
    const PerlinNoise &pn,
    const vec3 root_location
)
{
    auto defered_list = std::unordered_set<std::shared_ptr<Chunk>, ChunkHash, ChunkEqual>{ chunk };

    // Trunk
    for (int i = 1; i <= 6; ++i)
    {
        vec3 block_location = { root_location[0], root_location[1], root_location[2] + i };
        if (add_block(chunk, block_factory, BlockType::WOOD, block_location, true) == Result::OOB)
        {
            auto defered = add_block_relative_to_current(block_factory, pn, BlockType::WOOD, chunk->location, block_location);
            defered_list.insert(defered);
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
            if (add_block(chunk, block_factory, BlockType::LEAVES, block_location1, true) == Result::OOB)
            {
                auto defered = add_block_relative_to_current(block_factory, pn, BlockType::LEAVES, chunk->location, block_location1);
                defered_list.insert(defered);
            }
            vec3 block_location2 = { root_location[0] + x, root_location[1] + y, root_location[2] + 5 };
            if (add_block(chunk, block_factory, BlockType::LEAVES, block_location2, true) == Result::OOB)
            {
                auto defered = add_block_relative_to_current(block_factory, pn, BlockType::LEAVES, chunk->location, block_location2);
                defered_list.insert(defered);
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
            if (add_block(chunk, block_factory, BlockType::LEAVES, block_location, true) == Result::OOB)
            {
                auto defered = add_block_relative_to_current(block_factory, pn, BlockType::LEAVES, chunk->location, block_location);
                defered_list.insert(defered);
            }
        }
    }

    // Leaves (second 3x3 layer is plus-shaped)
    vec3 block_location1 = { root_location[0] + 0, root_location[1] + 0, root_location[2] + 7 };
    if (add_block(chunk, block_factory, BlockType::LEAVES, block_location1, true) == Result::OOB)
    {
        auto defered = add_block_relative_to_current(block_factory, pn, BlockType::LEAVES, chunk->location, block_location1);
        defered_list.insert(defered);
    }
    vec3 block_location2 = { root_location[0] + 1, root_location[1] + 0, root_location[2] + 7 };
    if (add_block(chunk, block_factory, BlockType::LEAVES, block_location2, true) == Result::OOB)
    {
        auto defered = add_block_relative_to_current(block_factory, pn, BlockType::LEAVES, chunk->location, block_location2);
        defered_list.insert(defered);
    }
    vec3 block_location3 = { root_location[0] - 1, root_location[1] + 0, root_location[2] + 7 };
    if (add_block(chunk, block_factory, BlockType::LEAVES, block_location3, true) == Result::OOB)
    {
        auto defered = add_block_relative_to_current(block_factory, pn, BlockType::LEAVES, chunk->location, block_location3);
        defered_list.insert(defered);
    }
    vec3 block_location4 = { root_location[0] + 0, root_location[1] + 1, root_location[2] + 7 };
    if (add_block(chunk, block_factory, BlockType::LEAVES, block_location4, true) == Result::OOB)
    {
        auto defered = add_block_relative_to_current(block_factory, pn, BlockType::LEAVES, chunk->location, block_location4);
        defered_list.insert(defered);
    }
    vec3 block_location5 = { root_location[0] + 0, root_location[1] - 1, root_location[2] + 7 };
    if (add_block(chunk, block_factory, BlockType::LEAVES, block_location5, true) == Result::OOB)
    {
        auto defered = add_block_relative_to_current(block_factory, pn, BlockType::LEAVES, chunk->location, block_location5);
        defered_list.insert(defered);
    }

    return defered_list;
}

void ChunkManager::update_mesh()
{
    bool chunk_update_pending = std::any_of(
        chunks.begin(),
        chunks.end(),
        [&](const std::shared_ptr<Chunk> &chunk)
        {
            return chunk->update_pending;
        }
    );

    if (chunk_update_pending)
    {
        terrain_mesh.vertices.clear();
        for (auto &chunk : chunks)
        {
            chunk->update_pending = false;
            if (!chunk->vertices.empty())
            {
                terrain_mesh.vertices.insert(terrain_mesh.vertices.end(), chunk->vertices.begin(), chunk->vertices.end());
            }
        }

        // Bind mesh to VBO
        glBindBuffer(GL_ARRAY_BUFFER, terrain_mesh.vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            terrain_mesh.vertices.size() * sizeof(BlockVertex),
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
std::shared_ptr<Chunk> ChunkManager::add_block_relative_to_current(
    const BlockFactory &block_factory,
    const PerlinNoise &pn,
    const BlockType type,
    const vec3 chunk_location,
    const vec3 block_location
)
{
    Settings &settings = Settings::get_instance();
    ssize_t chunk_size = settings.chunk_size;

    vec3 parent_chunk_location{};
    vec3 actual_block_location{};

    parent_chunk_location[0] = std::floorf(((chunk_location[0] * chunk_size) + block_location[0]) / (float)chunk_size);
    parent_chunk_location[1] = std::floorf(((chunk_location[1] * chunk_size) + block_location[1]) / (float)chunk_size);
    parent_chunk_location[2] = std::floorf(((chunk_location[2] * chunk_size) + block_location[2]) / (float)chunk_size);

    actual_block_location[0] = (float)(((int)block_location[0] % chunk_size + chunk_size) % chunk_size);
    actual_block_location[1] = (float)(((int)block_location[1] % chunk_size + chunk_size) % chunk_size);
    actual_block_location[2] = (float)(((int)block_location[2] % chunk_size + chunk_size) % chunk_size);

    // Spent waaaaaaay too much time figuring this edge case out. No clue why it works ¯\_(ツ)_/¯
    if (block_location[0] < 0.0f)
    {
        parent_chunk_location[0] += 2;
    }
    else if (block_location[0] >= 16.0f)
    {
        parent_chunk_location[0] -= 2;
    }

    auto lookup = std::make_shared<Chunk>(parent_chunk_location);
    auto needle = chunks.find(lookup);
    if (needle == chunks.end())
    {
        // Create new chunk
        ChunkFactory chunk_factory;
        auto new_chunk = chunk_factory.make_chunk(block_factory, pn, parent_chunk_location);
        chunks.insert(new_chunk);
        add_block(new_chunk, block_factory, type, actual_block_location, false);

        // TODO: move elsewhere
        // Generate the remaining chunks in the column
        //ssize_t base_biome_height = KC::SEA_LEVEL / chunk_size;
        //for (ssize_t i = base_biome_height; i < new_chunk->location[2]; ++i)
        //{
        //    vec3 tmp_chunk_location = { parent_chunk_location[0], parent_chunk_location[1], (float)i };
        //    auto tmp_chunk = chunk_factory.make_chunk(block_factory, pn, tmp_chunk_location);
        //    tmp_chunk->update_mesh();
        //    chunks.insert(tmp_chunk);
        //}

        return new_chunk;
    }
    else
    {
        // Use existing chunk
        std::shared_ptr<Chunk> chunk = *needle;
        add_block(chunk, block_factory, type, actual_block_location, false);
        return chunk;
    }
}
