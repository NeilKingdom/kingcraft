#pragma once

#include "common.hpp"
#include "settings.hpp"
#include "chunk_factory.hpp"

enum Result
{
    INVALID_ARG,    // Invalid argument
    OOB,            // Out-of-Bounds
    FAILURE,
    SUCCESS
};

typedef std::unordered_set<std::shared_ptr<Chunk>, ChunkHash, ChunkEqual> ChunkSet;

class ChunkManager
{
public:
    ChunkSet GCL; // Global Chunk List (list of chunks actively loaded in memory)
    std::vector<std::shared_ptr<Chunk>> chunk_cache; // Cache of chunks that player has edited
    std::vector<std::array<float, 2>> chunk_coords_2D{};
    Mesh<BlockVertex> terrain_mesh; // Mesh that encapsulates all interactable blocks

    // Special member functions
    ChunkManager(const ChunkManager &chunk_mgr) = delete;
    ChunkManager &operator=(const ChunkManager &chunk_mgr) = delete;
    ~ChunkManager();

    // General
    static ChunkManager &get_instance();

    Result add_block(
        std::shared_ptr<Chunk> &chunk,
        const BlockFactory &block_factory,
        const BlockType type,
        const vec3 block_location,
        const bool overwrite
    ) const;
    Result remove_block(std::shared_ptr<Chunk> &chunk, const vec3 block_location) const;
    //std::optional<std::shared_ptr<Chunk>> get_chunk(const Block &block) const;
    //std::optional<std::shared_ptr<Chunk>> get_chunk(const std::shared_ptr<Chunk> &chunk) const;
    ChunkSet plant_tree(
        std::shared_ptr<Chunk> &chunk,
        const BlockFactory &block_factory,
        const PerlinNoise &pn,
        const vec3 root_location
    );
    void update_mesh();

private:
    // Special member functions
    ChunkManager();

    // General
    std::shared_ptr<Chunk> add_block_relative_to_current(
        const BlockFactory &block_factory,
        const PerlinNoise &pn,
        const BlockType type,
        const vec3 chunk_location,
        const vec3 block_location
    );
};
