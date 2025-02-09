#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "block.hpp"

typedef std::array<float, 2> UvCoords;
static std::map<BlockType, std::optional<std::tuple<UvCoords, UvCoords, UvCoords>>> uv_cache;

class BlockFactory
{
public:
    // Special member functions
    BlockFactory() = default;
    ~BlockFactory() = default;

    // General
    static Block make_block(const BlockType type, const vec3 location, const uint8_t faces);
    static void populate_uv_cache();

private:
    // General
    static std::optional<std::tuple<UvCoords, UvCoords, UvCoords>> get_uv_coords(const BlockType type);
};
