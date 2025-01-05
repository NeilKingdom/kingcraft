#pragma once

#include "common.hpp"
#include "block_factory.hpp"
#include "game_state.hpp"
#include "chunk.hpp"
#include "noise_generator.hpp"

class ChunkFactory
{
public:
    // Special member functions
    ChunkFactory(const ChunkFactory &factory) = delete;
    ChunkFactory &operator=(const ChunkFactory &factory) = delete;
    ~ChunkFactory() = default;

    // General
    static ChunkFactory &get_instance();
    Chunk make_chunk(const vec3 location, const uint8_t sides) const;

private:
    // Special member functions
    ChunkFactory() = default;
};
