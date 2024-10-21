#pragma once

#include "texture.hpp"
#include "texture_atlas.hpp"
#include "constants.hpp"
#include "block.hpp"

static TextureAtlas atlas = TextureAtlas(
    KCConst::TEX_SIZE, KCConst::TEX_SIZE,
    "/home/neil/devel/projects/kingcraft/res/textures/texture_atlas.png"
);

// Dirt block
static Texture tex_dirt_top;
static Texture tex_dirt_sides;
static Texture tex_dirt_bottom;

// NOTE: Must be called after OpenGL context is created
static void init_textures()
{
    tex_dirt_top    = Texture(atlas.get_pixmap_at_id(0), GL_NEAREST, GL_NEAREST);
    tex_dirt_sides  = Texture(atlas.get_pixmap_at_id(1), GL_NEAREST, GL_NEAREST);
    tex_dirt_bottom = Texture(atlas.get_pixmap_at_id(2), GL_NEAREST, GL_NEAREST);
}

static std::optional<std::tuple<Texture, Texture, Texture>> get_tex_by_block_type(const BlockType type)
{
    std::optional<std::tuple<Texture, Texture, Texture>> textures;

    switch (type)
    {
        case BlockType::AIR:
            textures = std::nullopt;
            break;
        case BlockType::DIRT:
            textures = std::make_tuple(tex_dirt_top, tex_dirt_sides, tex_dirt_bottom);
            break;
        default:
            textures = std::nullopt;
            break;
    }

    return textures;
}
