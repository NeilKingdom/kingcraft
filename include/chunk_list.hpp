#pragma once

#include <set>
#include "chunk.hpp"

struct ChunkComparator
{
    bool operator()(const std::unique_ptr<Chunk> &a, const std::unique_ptr<Chunk> &b) const
    {
        // If (a.x == b.x) return (a.y < b.y) else return (a.x < b.x)
        return (a->location[0] == b->location[0]) ?
            (a->location[1] < b->location[1]) : (a->location[0] < b->location[0]);
    }
};

typedef std::set<std::unique_ptr<Chunk>, ChunkComparator> chunk_list_t;
