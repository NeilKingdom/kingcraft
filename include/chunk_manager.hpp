#pragma once

#include "common.hpp"
#include "settings.hpp"
#include "chunk_factory.hpp"
#include "chunk_set.hpp"

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
    ChunkSet GCL; // Global Chunk List (list of chunks actively loaded in memory)
    ChunkSet chunk_cache; // List of chunks that player has edited
    //std::vector<std::array<float, 2>> chunk_coords_2D{};
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
    ChunkSet plant_tree(std::shared_ptr<Chunk> &chunk, const vec3 root_location);
    void update_mesh();

private:
    // Special member functions
    ChunkManager();

    // General
    std::shared_ptr<Chunk> add_block_relative(
        const BlockType type,
        const vec3 chunk_location,
        const vec3 block_location
    );
};
