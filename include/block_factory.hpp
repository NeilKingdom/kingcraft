#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "block.hpp"

typedef std::array<float, 2> UvCoords;

class BlockFactory
{
public:
    // Special member functions
    BlockFactory();
    ~BlockFactory() = default;

    // General
    Block make_block(const BlockType type, const vec3 location, const uint8_t faces) const;

private:
    std::map<BlockType, std::optional<std::tuple<UvCoords, UvCoords, UvCoords>>> uv_cache;

    // General
    std::optional<std::tuple<UvCoords, UvCoords, UvCoords>> get_uv_coords(const BlockType type);
};
