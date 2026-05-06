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
    ChunkMap GCL;         // Global Chunk List (list of chunks actively loaded in memory)
    ChunkMap chunk_cache; // List of chunks that player has edited
    Mesh terrain_mesh;    // Mesh that encapsulates all interactable blocks

    // Special member functions
    ChunkManager(const ChunkManager &chunk_mgr) = delete;
    ChunkManager &operator=(const ChunkManager &chunk_mgr) = delete;
    ChunkManager(ChunkManager &&chunk_mgr) = delete;
    ChunkManager &operator=(ChunkManager &&chunk_mgr) = delete;

    // General
    static ChunkManager &get_instance();

    //std::optional<Block&> get_block(const Vec3_t world_location) const;
    Result add_block(
        std::shared_ptr<Chunk> &chunk,
        const BlockType type,
        const Vec3_t block_location,
        const bool overwrite = false
    ) const;
    Result remove_block(std::shared_ptr<Chunk> &chunk, const Vec3_t block_location) const;
    ChunkMap plant_tree(std::shared_ptr<Chunk> &chunk, const Vec3_t root_location);
    ChunkMap plant_trees(std::shared_ptr<Chunk> &chunk, const float density = 0.0033f);
    void bind_terrain_mesh();

private:
    // Special member functions
    ChunkManager();
    ~ChunkManager();

    // General
    void get_relative_locations(
        const Vec3_t &chunk_location,
        const Vec3_t &block_location,
        Vec3_t &actual_chunk_location,
        Vec3_t &actual_block_location
    ) const;
    std::shared_ptr<Chunk> add_block_relative(
        std::shared_ptr<Chunk> &chunk,
        const BlockType type,
        const Vec3_t block_location
    );
};
