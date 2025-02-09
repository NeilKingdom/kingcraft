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
    ChunkManager();
    ~ChunkManager() = default;

    // General
    std::optional<std::shared_ptr<Chunk>> get_chunk_at_location_ci(const vec3 location) const;
    std::optional<std::shared_ptr<Chunk>> get_chunk_at_location_bi(const vec3 location) const;

    static void add_block(std::shared_ptr<Chunk> &chunk, const BlockType type, const vec3 location);
    static void remove_block(std::shared_ptr<Chunk> &chunk, const vec3 location);
};
