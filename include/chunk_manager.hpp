#pragma once

#include "common.hpp"
#include "settings.hpp"
#include "chunk_factory.hpp"
#include "chunk_map.hpp"

// TODO: Don't like
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
    ChunkMap GCL; // Global Chunk List (list of chunks actively loaded in memory)
    ChunkMap chunk_cache; // List of chunks that player has edited
    Mesh<VPosTex> terrain_mesh; // Mesh that encapsulates all interactable blocks

    // Special member functions
    ChunkManager(const ChunkManager &chunk_mgr) = delete;
    ChunkManager &operator=(const ChunkManager &chunk_mgr) = delete;
    ~ChunkManager();

    // General
    static ChunkManager &get_instance();

    Result add_block(
        std::shared_ptr<Chunk> &chunk,
        const BlockType type,
        const vec3 block_location,
        const bool overwrite
    ) const;
    Result remove_block(std::shared_ptr<Chunk> &chunk, const vec3 block_location) const;
    ChunkMap plant_tree(std::shared_ptr<Chunk> &chunk, const vec3 root_location);
    // TODO: Add likeliness as default param
    ChunkMap plant_trees(std::shared_ptr<Chunk> &chunk);
    void update_mesh();

private:
    // Special member functions
    ChunkManager();

    // General
    std::shared_ptr<Chunk> add_block_relative(
        std::shared_ptr<Chunk> &chunk,
        const BlockType type,
        const vec3 block_location
    );
};
