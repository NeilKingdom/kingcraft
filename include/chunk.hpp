#pragma once

#include "common.hpp"
#include "block.hpp"
#include "block_factory.hpp"
#include "settings.hpp"
#include "utils.hpp"

class Chunk
{
public:
    vec3 location;
    bool update_pending;
    std::weak_ptr<Chunk> tree_ref;
    std::vector<VPosTex> vertices;
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

