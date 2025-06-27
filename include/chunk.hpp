#pragma once

#include "common.hpp"
#include "block.hpp"
#include "block_factory.hpp"
#include "settings.hpp"
#include "helpers.hpp"

class Chunk
{
public:
    vec3 location;
    bool update_pending;
    std::vector<BlockVertex> vertices;
    std::vector<std::vector<uint8_t>> block_heights;
    std::vector<std::vector<std::vector<Block>>> blocks;

    // Special member functions
    Chunk();
    Chunk(const vec3 location);
    ~Chunk() = default;

    // General
    void update_mesh();
    bool operator==(const Chunk &chunk) const;
};

struct ChunkHash
{
    std::size_t operator()(const std::shared_ptr<Chunk> &chunk) const
    {
        std::size_t h1 = std::hash<ssize_t>{}(chunk->location[0]);
        std::size_t h2 = std::hash<ssize_t>{}(chunk->location[1]);
        std::size_t h3 = std::hash<ssize_t>{}(chunk->location[2]);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

struct ChunkEqual
{
    bool operator()(const std::shared_ptr<Chunk> &a, const std::shared_ptr<Chunk> &b) const
    {
        return *a == *b;
    }
};

