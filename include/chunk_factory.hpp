#pragma once

#include "common.hpp"
#include "chunk.hpp"
#include "block_factory.hpp"
#include "settings.hpp"
#include "perlin_noise.hpp"

class ChunkFactory
{
public:
    // Special member functions
    ChunkFactory() = default;
    ~ChunkFactory() = default;

    // General
    std::shared_ptr<Chunk> make_chunk(
        const BlockFactory &block_factory,
        const PerlinNoise &pn,
        const vec3 chunk_location,
        const bool is_tallest_in_col
    ) const;
    std::vector<std::shared_ptr<Chunk>> make_chunk_column(
        const BlockFactory &block_factory,
        const PerlinNoise &pn,
        const vec2 chunk_col_location
    ) const;
};
