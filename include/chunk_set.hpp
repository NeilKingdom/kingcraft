#pragma once

#include "common.hpp"
#include "chunk.hpp"

struct ChunkHash {
    size_t operator()(const std::shared_ptr<Chunk> &chunk) const
    {
        size_t hash = 0;
        for (int i = 0; i < 3; ++i)
        {
            // Bit cast will translate bits to uint32_t instead of truncating decimal
            uint32_t bits = std::bit_cast<uint32_t>(chunk->location[i]);
            hash ^= std::hash<uint32_t>{}(bits)
                + 0X9E3779B97F4A7C15ULL
                + (hash << 6)
                + (hash >> 2);
        }

        return hash;
    }
};

struct ChunkEqual
{
    bool operator()(const std::shared_ptr<Chunk> &a, const std::shared_ptr<Chunk> &b) const
    {
        // Chunk has override for == operator that uses V3_EQ()
        return *a == *b;
    }
};

typedef std::unordered_set<std::shared_ptr<Chunk>, ChunkHash, ChunkEqual> ChunkSet;
