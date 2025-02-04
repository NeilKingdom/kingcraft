#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "block.hpp"

typedef std::array<float, 2> UvCoords;

class BlockFactory
{
public:
    // Special member functions
    BlockFactory(const BlockFactory &factory) = delete;
    BlockFactory &operator=(const BlockFactory &factory) = delete;
    ~BlockFactory() = default;

    // General
    static BlockFactory &get_instance();
    Block make_block(const BlockType type, const vec3 location, const uint8_t faces);

private:
    std::map<BlockType, std::optional<std::tuple<UvCoords, UvCoords, UvCoords>>> m_uv_cache;

    // Special member functions
    BlockFactory() = default;

    // General
    std::optional<std::tuple<UvCoords, UvCoords, UvCoords>> get_uv_coords(const BlockType type) const;
    void populate_uv_cache();
};
