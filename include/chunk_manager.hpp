#pragma once

#include "common.hpp"
#include "chunk.hpp"

class ChunkManager
{
public:
    std::vector<std::shared_ptr<Chunk>> chunks;             // List of chunks that are actively loaded in memory
    std::vector<std::shared_ptr<Chunk>> chunk_cache;        // Cache of chunks that player has edited
    std::vector<std::array<float, 2>> chunk_col_coords;     // List of chunk column (x, y) coordinates that are visible within the camera's frustum

    // Special member functions
    ChunkManager(const ChunkManager &chunk_mgr) = delete;
    ChunkManager &operator=(const ChunkManager &chunk_mgr) = delete;
    ~ChunkManager() = default;

    // General
    static ChunkManager &get_instance();
    std::optional<std::shared_ptr<Chunk>> get_chunk_at_location(const vec3 location) const;

private:
    // Special member functions
    ChunkManager() = default;
};
