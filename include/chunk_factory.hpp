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
    std::shared_ptr<Chunk> make_chunk(PerlinNoise &pn, const vec3 location, const bool is_tallest_in_col) const;
    std::vector<std::shared_ptr<Chunk>> make_chunk_column(PerlinNoise &pn, const vec2 location) const;
};
