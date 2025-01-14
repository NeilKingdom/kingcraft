#pragma once

#include "common.hpp"
#include "chunk.hpp"
#include "chunk_factory.hpp"
#include "block_factory.hpp"

class ChunkColumn
{
public:
    vec3 location;
    std::vector<std::shared_ptr<Chunk>> chunk_col;

    ChunkColumn() = default;
    ~ChunkColumn() = default;

    bool operator==(const ChunkColumn &chunk_col) const;
    bool operator<(const ChunkColumn &chunk_col) const;
};

ChunkColumn make_chunk_column(vec2 location);
