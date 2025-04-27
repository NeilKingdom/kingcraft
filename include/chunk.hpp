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
    bool updated;
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
