#pragma once

#include "common.hpp"
#include "block_factory.hpp"
#include "game.hpp"
#include "settings.hpp"
#include "chunk_manager.hpp"
#include "perlin_noise.hpp"

class ChunkFactory
{
public:
    // Special member functions
    ChunkFactory() = default;
    ~ChunkFactory() = default;

    // General
    std::shared_ptr<Chunk> make_chunk(PerlinNoise &pn, const vec3 location, const uint8_t sides) const;
    std::vector<std::shared_ptr<Chunk>> make_chunk_column(PerlinNoise &pn, const vec2 location) const;

private:
    // General
    size_t hash_coord_to_range(const vec3 location, const size_t min, const size_t max) const;
};
