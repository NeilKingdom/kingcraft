#pragma once

#include "common.hpp"
#include "block_factory.hpp"
#include "game_state.hpp"
#include "chunk_manager.hpp"

class ChunkFactory
{
public:
    // Special member functions
    ChunkFactory() = default;
    ~ChunkFactory() = default;

    // General
    static std::shared_ptr<Chunk> make_chunk(const vec3 location, const uint8_t sides);
    static std::vector<std::shared_ptr<Chunk>> make_chunk_column(const vec2 location);
    static void plant_tree(std::shared_ptr<Chunk> &chunk, const vec3 sprout_location);
};
