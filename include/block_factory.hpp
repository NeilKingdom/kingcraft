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
    void init();
    Block make_block(const BlockType type, const mat4 m_trns, const uint8_t sides);

private:
    Texture m_block_tex;

    // Special member functions
    BlockFactory() = default;

    // General
    std::optional<std::tuple<UvCoords, UvCoords, UvCoords>> get_uv_coords(const BlockType type) const;
};
