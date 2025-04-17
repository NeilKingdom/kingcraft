#pragma once

#include "common.hpp"
#include "chunk.hpp"
#include "settings.hpp"

enum Result
{
    INVALID_ARG,    // Invalid argument
    OOB,            // Out-of-Bounds
    FAILURE,
    SUCCESS
};

class ChunkManager
{
public:
    Mesh<BlockVertex> terrain_mesh;
    std::vector<std::shared_ptr<Chunk>> chunks;         // List of chunks that are actively loaded in memory
    std::vector<std::shared_ptr<Chunk>> chunk_cache;    // Cache of chunks that player has edited
    std::vector<std::array<float, 2>> chunk_col_coords; // List of chunk column (x, y) coordinates that are visible within the camera's frustum

    // Special member functions
    ChunkManager(const ChunkManager &chunk_mgr) = delete;
    ChunkManager &operator=(const ChunkManager &chunk_mgr) = delete;
    ~ChunkManager();

    // General
    static ChunkManager &get_instance();

    Result add_block(const BlockFactory &block_factory, std::shared_ptr<Chunk> &chunk, const BlockType type, const vec3 location, const bool overwrite);
    Result remove_block(std::shared_ptr<Chunk> &chunk, const vec3 location);
    std::optional<std::shared_ptr<Chunk>> get_chunk(const Block &block) const;
    std::optional<std::shared_ptr<Chunk>> get_chunk(const Chunk &chunk) const;
    void plant_tree(const BlockFactory &block_factory, std::shared_ptr<Chunk> &chunk, const vec3 location);
    void update_mesh();

private:
    // Special member functions
    ChunkManager();

    // General
    void add_block_relative_to_current(const BlockFactory &block_factory, const vec3 &chunk_location, const vec3 &block_location);
};
