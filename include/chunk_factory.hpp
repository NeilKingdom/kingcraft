#pragma once

#include "common.hpp"
#include "chunk.hpp"
#include "block_factory.hpp"
#include "settings.hpp"
#include "biome.hpp"

class ChunkFactory
{
public:
    // Special member functions
    ~ChunkFactory() = default;

    // General
    static ChunkFactory &get_instance();

    std::shared_ptr<Chunk> make_chunk(const vec3 chunk_location) const;

private:
    ChunkFactory() = default;
};
