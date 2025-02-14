#pragma once

#include "common.hpp"
#include "chunk.hpp"
#include "settings.hpp"

class ChunkManager
{
public:
    std::vector<std::shared_ptr<Chunk>> chunks;         // List of chunks that are actively loaded in memory
    std::vector<std::shared_ptr<Chunk>> chunk_cache;    // Cache of chunks that player has edited
    std::vector<std::array<float, 2>> chunk_col_coords; // List of chunk column (x, y) coordinates that are visible within the camera's frustum

    // Special member functions
    ChunkManager(const ChunkManager &chunk_mgr) = delete;
    ChunkManager &operator=(const ChunkManager &chunk_mgr) = delete;
    ~ChunkManager() = default;

    // General
    static ChunkManager &get_instance();

    bool add_block(std::shared_ptr<Chunk> &chunk, const BlockType type, const vec3 location, const bool overwrite);
    bool remove_block(std::shared_ptr<Chunk> &chunk, const vec3 location);
    void plant_tree(std::shared_ptr<Chunk> &chunk, const vec3 location);
    std::optional<std::shared_ptr<Chunk>> get_chunk_at_block_offset(const vec3 location) const;
    std::optional<std::shared_ptr<Chunk>> get_chunk_at_chunk_offset(const vec3 location) const;

private:
    // Special member functions
    ChunkManager() = default;
};
