/**
 * @file block_factory.cpp
 * @author Neil Kingdom
 * @since 16-10-2024
 * @version 1.0
 * @brief A factory class for creating blocks.
 */

#include "block_factory.hpp"

/**
 * @brief Default constructor for BlockFactory class.
 * @since 13-02-2025
 */
BlockFactory::BlockFactory()
{
    // Populate UV cache
    uv_cache[BlockType::DIRT]       = get_uv_coords(BlockType::DIRT);
    uv_cache[BlockType::GRASS]      = get_uv_coords(BlockType::GRASS);
    uv_cache[BlockType::WOOD]       = get_uv_coords(BlockType::WOOD);
    uv_cache[BlockType::LEAVES]     = get_uv_coords(BlockType::LEAVES);
    uv_cache[BlockType::SAND]       = get_uv_coords(BlockType::SAND);
    uv_cache[BlockType::STONE]      = get_uv_coords(BlockType::STONE);
    // TODO: Finish...
}

BlockFactory &BlockFactory::get_instance()
{
    static BlockFactory block_factory;
    return block_factory;
}

/**
 * @brief Returns a tuple of UV coordinates for each face of the block based on __type__.
 * @since 24-10-2024
 * @param[in] type The block type that we are retrieving UV coordinates for
 * @returns Optionally returns a tuple of UV coordinates for the top, sides, and bottom of the block, respectively
 */
std::optional<std::tuple<UV, UV, UV>>
BlockFactory::get_uv_coords(const BlockType type) const
{
    float tx_offset, ty_offset;
    UV uv_top, uv_sides, uv_bottom;

    switch (type)
    {
        case BlockType::DIRT:
            ty_offset = 0.0f;
            tx_offset = 2.0f;

            // Top + Sides + Bottom
            uv_top.u = uv_sides.u = uv_bottom.u = tx_offset / (float)KC::TEX_ATLAS_NCOLS;
            uv_top.v = uv_sides.v = uv_bottom.v = ty_offset / (float)KC::TEX_ATLAS_NCOLS;
            break;
        case BlockType::GRASS:
            ty_offset = 0.0f;

            // Top
            tx_offset = 0.0f;
            uv_top.u = tx_offset / (float)KC::TEX_ATLAS_NCOLS;
            uv_top.v = ty_offset / (float)KC::TEX_ATLAS_NCOLS;

            // Sides
            tx_offset = 1.0f;
            uv_sides.u = tx_offset / (float)KC::TEX_ATLAS_NCOLS;
            uv_sides.v = ty_offset / (float)KC::TEX_ATLAS_NCOLS;

            // Bottom
            tx_offset = 2.0f;
            uv_bottom.u = tx_offset / (float)KC::TEX_ATLAS_NCOLS;
            uv_bottom.v = ty_offset / (float)KC::TEX_ATLAS_NCOLS;
            break;
        case BlockType::WOOD:
            ty_offset = 0.0f;

            // Top + Bottom
            tx_offset = 3.0f;
            uv_top.u = uv_bottom.u = tx_offset / (float)KC::TEX_ATLAS_NCOLS;
            uv_top.v = uv_bottom.v = ty_offset / (float)KC::TEX_ATLAS_NCOLS;

            // Sides
            tx_offset = 4.0f;
            uv_sides.u = tx_offset / (float)KC::TEX_ATLAS_NCOLS;
            uv_sides.v = ty_offset / (float)KC::TEX_ATLAS_NCOLS;
            break;
        case BlockType::LEAVES:
            ty_offset = 0.0f;
            tx_offset = 5.0f;

            // Top + Sides + Bottom
            uv_top.u = uv_sides.u = uv_bottom.u = tx_offset / (float)KC::TEX_ATLAS_NCOLS;
            uv_top.v = uv_sides.v = uv_bottom.v = ty_offset / (float)KC::TEX_ATLAS_NCOLS;
            break;
        case BlockType::SAND:
            ty_offset = 1.0f;
            tx_offset = 0.0f;

            // Top + Sides + Bottom
            uv_top.u = uv_sides.u = uv_bottom.u = tx_offset / (float)KC::TEX_ATLAS_NCOLS;
            uv_top.v = uv_sides.v = uv_bottom.v = ty_offset / (float)KC::TEX_ATLAS_NCOLS;
            break;
        case BlockType::STONE:
            ty_offset = 1.0f;
            tx_offset = 1.0f;

            // Top + Sides + Bottom
            uv_top.u = uv_sides.u = uv_bottom.u = tx_offset / (float)KC::TEX_ATLAS_NCOLS;
            uv_top.v = uv_sides.v = uv_bottom.v = ty_offset / (float)KC::TEX_ATLAS_NCOLS;
            break;
        default:
            return std::nullopt;
    }

    return std::make_optional(std::make_tuple(uv_top, uv_sides, uv_bottom));
}

/**
 * @brief Creates a single block based on the given parameters.
 * @since 16-10-2024
 * @param[in] type The block type of the block being created e.g., dirt, grass, etc.
 * @param[in] faces A mask which determines which sides of the block will be rendered
 * @param[in] world_location A vec3 which determines the location of the block relative to the world origin
 * @returns A Block object which matches the requested attributes
 */
Block BlockFactory::make_block(
    const BlockType type,
    const uint8_t faces,
    const Vec3_t world_location
) const
{
    if (faces == 0 || type == BlockType::AIR)
    {
        return Block();
    }

    Block block = Block(type, faces);

    // UV coordinates
    constexpr float uv_pad = 0.005f;
    constexpr float tw = (1.0f / KC::TEX_ATLAS_NCOLS) - uv_pad;
    constexpr float th = (1.0f / KC::TEX_ATLAS_NCOLS) - uv_pad;

    // TODO: Something breaks when invalid block type specified...
    auto uv = uv_cache.at(type).value_or(
        std::make_tuple(UV{}, UV{}, UV{})
    );

    UV uv_top    = std::get<0>(uv);
    UV uv_sides  = std::get<1>(uv);
    UV uv_bottom = std::get<2>(uv);

    /*
     * Vertex positions
     *
     *   4____5
     *  /|   /|
     * 0-+--1 |
     * | 6__|_7
     * |/   |/
     * 2----3
     */
    Vec3_t v0 = { .v = { -0.5f + world_location.x, -0.5f + world_location.y,  0.5f + world_location.z }};
    Vec3_t v1 = { .v = { -0.5f + world_location.x,  0.5f + world_location.y,  0.5f + world_location.z }};
    Vec3_t v2 = { .v = { -0.5f + world_location.x, -0.5f + world_location.y, -0.5f + world_location.z }};
    Vec3_t v3 = { .v = { -0.5f + world_location.x,  0.5f + world_location.y, -0.5f + world_location.z }};
    Vec3_t v4 = { .v = {  0.5f + world_location.x, -0.5f + world_location.y,  0.5f + world_location.z }};
    Vec3_t v5 = { .v = {  0.5f + world_location.x,  0.5f + world_location.y,  0.5f + world_location.z }};
    Vec3_t v6 = { .v = {  0.5f + world_location.x, -0.5f + world_location.y, -0.5f + world_location.z }};
    Vec3_t v7 = { .v = {  0.5f + world_location.x,  0.5f + world_location.y, -0.5f + world_location.z }};

    block.right_face = {
        VPosTex{ v1.x, v1.y, v1.z, uv_sides.u + uv_pad, uv_sides.v + uv_pad },
        VPosTex{ v7.x, v7.y, v7.z, uv_sides.u + tw,     uv_sides.v + th },
        VPosTex{ v3.x, v3.y, v3.z, uv_sides.u + uv_pad, uv_sides.v + th },
        VPosTex{ v7.x, v7.y, v7.z, uv_sides.u + tw,     uv_sides.v + th },
        VPosTex{ v1.x, v1.y, v1.z, uv_sides.u + uv_pad, uv_sides.v + uv_pad },
        VPosTex{ v5.x, v5.y, v5.z, uv_sides.u + tw,     uv_sides.v + uv_pad }
    };

    block.left_face = {
        VPosTex{ v4.x, v4.y, v4.z, uv_sides.u + uv_pad, uv_sides.v + uv_pad },
        VPosTex{ v2.x, v2.y, v2.z, uv_sides.u + tw,     uv_sides.v + th },
        VPosTex{ v6.x, v6.y, v6.z, uv_sides.u + uv_pad, uv_sides.v + th },
        VPosTex{ v2.x, v2.y, v2.z, uv_sides.u + tw,     uv_sides.v + th },
        VPosTex{ v4.x, v4.y, v4.z, uv_sides.u + uv_pad, uv_sides.v + uv_pad },
        VPosTex{ v0.x, v0.y, v0.z, uv_sides.u + tw,     uv_sides.v + uv_pad }
    };

    block.front_face = {
        VPosTex{ v0.x, v0.y, v0.z, uv_sides.u + uv_pad, uv_sides.v + uv_pad },
        VPosTex{ v3.x, v3.y, v3.z, uv_sides.u + tw,     uv_sides.v + th },
        VPosTex{ v2.x, v2.y, v2.z, uv_sides.u + uv_pad, uv_sides.v + th },
        VPosTex{ v3.x, v3.y, v3.z, uv_sides.u + tw,     uv_sides.v + th },
        VPosTex{ v0.x, v0.y, v0.z, uv_sides.u + uv_pad, uv_sides.v + uv_pad },
        VPosTex{ v1.x, v1.y, v1.z, uv_sides.u + tw,     uv_sides.v + uv_pad }
    };

    block.back_face = {
        VPosTex{ v5.x, v5.y, v5.z, uv_sides.u + uv_pad, uv_sides.v + uv_pad },
        VPosTex{ v6.x, v6.y, v6.z, uv_sides.u + tw,     uv_sides.v + th },
        VPosTex{ v7.x, v7.y, v7.z, uv_sides.u + uv_pad, uv_sides.v + th },
        VPosTex{ v6.x, v6.y, v6.z, uv_sides.u + tw,     uv_sides.v + th },
        VPosTex{ v5.x, v5.y, v5.z, uv_sides.u + uv_pad, uv_sides.v + uv_pad },
        VPosTex{ v4.x, v4.y, v4.z, uv_sides.u + tw,     uv_sides.v + uv_pad }
    };

    block.bottom_face = {
        VPosTex{ v2.x, v2.y, v2.z, uv_bottom.u + uv_pad, uv_bottom.v + th },
        VPosTex{ v3.x, v3.y, v3.z, uv_bottom.u + tw,     uv_bottom.v + th },
        VPosTex{ v6.x, v6.y, v6.z, uv_bottom.u + uv_pad, uv_bottom.v + uv_pad },
        VPosTex{ v7.x, v7.y, v7.z, uv_bottom.u + tw,     uv_bottom.v + uv_pad },
        VPosTex{ v6.x, v6.y, v6.z, uv_bottom.u + uv_pad, uv_bottom.v + uv_pad },
        VPosTex{ v3.x, v3.y, v3.z, uv_bottom.u + tw,     uv_bottom.v + th }
    };

    block.top_face = {
        VPosTex{ v4.x, v4.y, v4.z, uv_top.u + uv_pad, uv_top.v + uv_pad },
        VPosTex{ v1.x, v1.y, v1.z, uv_top.u + tw,     uv_top.v + th },
        VPosTex{ v0.x, v0.y, v0.z, uv_top.u + uv_pad, uv_top.v + th },
        VPosTex{ v1.x, v1.y, v1.z, uv_top.u + tw,     uv_top.v + th },
        VPosTex{ v4.x, v4.y, v4.z, uv_top.u + uv_pad, uv_top.v + uv_pad },
        VPosTex{ v5.x, v5.y, v5.z, uv_top.u + tw,     uv_top.v + uv_pad }
    };

    return block;
}
