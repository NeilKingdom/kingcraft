#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "block.hpp"

typedef std::array<float, 2> UvCoords;

class BlockFactory
{
public:
    // Special member functions
    BlockFactory(const BlockFactory&) = delete;
    BlockFactory &operator=(const BlockFactory&) = delete;
    ~BlockFactory() = default;

    // General
    static BlockFactory &get_instance();
    Block make_block(const BlockType type, const mat4 m_trns, const Face sides) const;

private:
    // Special member functions
    BlockFactory();

    // General
    std::optional<std::tuple<UvCoords, UvCoords, UvCoords>> get_tex_by_block_type(const BlockType type) const;
};
