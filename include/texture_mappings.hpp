#pragma once

#include "texture.hpp"
#include "texture_atlas.hpp"
#include "constants.hpp"
#include "block.hpp"

typedef std::array<float, 2> UvCoords;

static std::optional<std::tuple<UvCoords, UvCoords, UvCoords>> get_tex_by_block_type(const BlockType type)
{
    std::optional<std::tuple<UvCoords, UvCoords, UvCoords>> textures;

    unsigned row, col;
    UvCoords uv_top, uv_sides, uv_bottom;

    switch (type)
    {
        case BlockType::AIR:
            textures = std::nullopt;
            break;
        case BlockType::DIRT:
            // Top
            uv_top[0] = 0.0f / 16.0f;
            uv_top[1] = 0.0f;

            // Sides
            uv_sides[0] = 1.0f / 16.0f;
            uv_sides[1] = 0.0f;

            // Bottom
            uv_bottom[0] = 2.0f / 16.0f;
            uv_bottom[1] = 0.0f;

            textures = std::make_tuple(uv_top, uv_sides, uv_bottom);
            break;
        default:
            textures = std::nullopt;
            break;
    }

    return textures;
}
