#pragma once

#include "common.hpp"
#include "block.hpp"
#include "block_factory.hpp"
#include "settings.hpp"
#include "mesh.hpp"

class Chunk
{
public:
    Mesh mesh;
    vec3 location;
    uint8_t faces;
    std::vector<std::vector<std::vector<Block>>> blocks;

    // Special member functions
    Chunk() = default;
    ~Chunk();

    // General
    void squash_block_meshes();
    bool operator==(const Chunk &chunk) const;

private:
    // General
    void delete_vao_and_vbo();
};
