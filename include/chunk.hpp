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
    bool is_tallest_in_col;
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
    std::string get_string_hash() const;
    bool operator==(const Chunk &chunk) const;
};

struct ChunkHash
{
    std::size_t operator()(const std::shared_ptr<Chunk> &chunk) const
    {
        return std::hash<std::string>()(chunk->get_string_hash());
    }
};

struct ChunkEqual
{
    bool operator()(const std::shared_ptr<Chunk> &a, const std::shared_ptr<Chunk> &b) const
    {
        return *a == *b;
    }
};

