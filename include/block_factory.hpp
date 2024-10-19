#pragma once

#include "common.hpp"
#include "texture_atlas.hpp"
#include "block.hpp"

class BlockFactory
{
public:
    // Special member functions
    BlockFactory(const BlockFactory&) = delete;
    BlockFactory &operator=(const BlockFactory&) = delete;
    ~BlockFactory() = default;

    // General
    static BlockFactory &get_instance();
    Block make_block(const BlockType type, const mat4 m_trns, const uint8_t sides) const;

private:
    // Special member functions
    BlockFactory();
};
