#pragma once

#include "common.hpp"
#include "block_factory.hpp"
#include "game_state.hpp"
#include "chunk.hpp"

class ChunkFactory
{
public:
    // Special member functions
    ChunkFactory(const ChunkFactory&) = delete;
    ChunkFactory &operator=(const ChunkFactory&) = delete;
    ~ChunkFactory() = default;

    // General
    static ChunkFactory &get_instance();
    Chunk make_chunk(const mat4 &m_trns, const uint8_t sides) const;

private:
    // Special member functions
    ChunkFactory() = default;
};
