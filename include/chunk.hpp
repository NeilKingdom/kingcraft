#pragma once

#include "common.hpp"
#include "block.hpp"
#include "block_factory.hpp"
#include "settings.hpp"
#include "utils.hpp"

class Chunk
{
public:
    // Member variables
    Vec3_t location;
    bool update_pending;
    std::weak_ptr<Chunk> tree_ref;
    std::vector<Vertex> vertices;
    std::vector<std::vector<uint8_t>> block_heights;
    std::vector<std::vector<std::vector<Block>>> blocks;

    // Special member functions
    Chunk();
    Chunk(const Vec3_t location);
    ~Chunk() = default;
    Chunk(const Chunk &chunk) = default;
    Chunk &operator=(const Chunk &chunk) = default;
    Chunk(Chunk &&chunk) = default;
    Chunk &operator=(Chunk &&chunk) = default;

    // General
    void update_mesh();
    bool operator==(const Chunk &chunk) const;
};

