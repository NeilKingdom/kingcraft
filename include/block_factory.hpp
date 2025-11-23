#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "block.hpp"

typedef std::array<float, 2> UvCoords;

class BlockFactory
{
public:
    // Special member functions
    ~BlockFactory() = default;

    // General
    static BlockFactory &get_instance();

    Block make_block(const BlockType type, const uint8_t faces, const vec3 world_location) const;

private:
    std::map<BlockType, std::optional<std::tuple<UvCoords, UvCoords, UvCoords>>> uv_cache;

    // Special member functions
    BlockFactory();

    // General
    std::optional<std::tuple<UvCoords, UvCoords, UvCoords>> get_uv_coords(const BlockType type) const;
};
