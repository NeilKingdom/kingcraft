/**
 * @file block_factory.cpp
 * @author Neil Kingdom
 * @since 16-10-2024
 * @version 1.0
 * @brief A singleton class which constructs Block objects.
 */

#include "block_factory.hpp"

/**
 * @brief Returns the single instance of BlockFactory.
 * @since 16-10-2024
 * @returns The BlockFactory instance
 */
BlockFactory &BlockFactory::get_instance()
{
    static BlockFactory instance;
    instance.populate_uv_cache();
    return instance;
}

/**
 * @brief Returns a tuple of UV coordinates for each face of the block based on __type__.
 * @since 24-10-2024
 * @param[in] type The block type that we are retrieving UV coordinates for
 * @returns Optionally returns a tuple of UV coordinates for the top, sides, and bottom of the block, respectively
 */
std::optional<std::tuple<UvCoords, UvCoords, UvCoords>>
BlockFactory::get_uv_coords(const BlockType type) const
{
    unsigned row, col;
    float tx_offset, ty_offset;
    UvCoords uv_top, uv_sides, uv_bottom;

    switch (type)
    {
        case BlockType::DIRT:
            ty_offset = 0.0f;
            tx_offset = 2.0f;

            uv_top[0] = uv_sides[0] = uv_bottom[0] = tx_offset / (float)KC::TEX_ATLAS_NCOLS;
            uv_top[1] = uv_sides[1] = uv_bottom[1] = ty_offset;
            break;
        case BlockType::GRASS:
            ty_offset = 0.0f;

            // Top
            tx_offset = 0.0f;
            uv_top[0] = tx_offset / (float)KC::TEX_ATLAS_NCOLS;
            uv_top[1] = ty_offset;

            // Sides
            tx_offset = 1.0f;
            uv_sides[0] = tx_offset / (float)KC::TEX_ATLAS_NCOLS;
            uv_sides[1] = ty_offset;

            // Bottom
            tx_offset = 2.0f;
            uv_bottom[0] = tx_offset / (float)KC::TEX_ATLAS_NCOLS;
            uv_bottom[1] = ty_offset;
            break;
        default:
            return std::nullopt;
    }

    return std::make_tuple(uv_top, uv_sides, uv_bottom);
}

void BlockFactory::populate_uv_cache()
{
    m_uv_cache[BlockType::GRASS] = get_uv_coords(BlockType::GRASS);
    // TODO: ...
}

/**
 * @brief Creates a single block based on a set of given parameters.
 * @since 16-10-2024
 * @param[in] type The block type of the block being created e.g. dirt, grass, etc.
 * @param[in] location A vec3 which determines the location of the block relative to the parent chunk
 * @param[in] sides A mask which determines which sides of the block will be rendered
 * @returns A Block object with the specified attributes
 */
Block BlockFactory::make_block(
    const BlockType type,
    const vec3 location,
    const uint8_t faces
)
{
    if (faces == 0 || type == BlockType::AIR)
    {
        return Block(BlockType::AIR);
    }

    Block block = Block(type);
    block.faces = faces;

    // UV coordinates
    constexpr float uv_pad = 0.005f;
    constexpr float uw = (1.0f / KC::TEX_ATLAS_NCOLS) - uv_pad;
    constexpr float vh = (1.0f / KC::TEX_ATLAS_NCOLS) - uv_pad;

    auto uv = m_uv_cache[type].value_or(
        std::make_tuple(UvCoords{}, UvCoords{}, UvCoords{})
    );

    UvCoords uv_top    = std::get<0>(uv);
    UvCoords uv_sides  = std::get<1>(uv);
    UvCoords uv_bottom = std::get<2>(uv);

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
    vec3 v0 = { -0.5f + location[0], -0.5f + location[1],  0.5f + location[2] };
    vec3 v1 = { -0.5f + location[0],  0.5f + location[1],  0.5f + location[2] };
    vec3 v2 = { -0.5f + location[0], -0.5f + location[1], -0.5f + location[2] };
    vec3 v3 = { -0.5f + location[0],  0.5f + location[1], -0.5f + location[2] };
    vec3 v4 = {  0.5f + location[0], -0.5f + location[1],  0.5f + location[2] };
    vec3 v5 = {  0.5f + location[0],  0.5f + location[1],  0.5f + location[2] };
    vec3 v6 = {  0.5f + location[0], -0.5f + location[1], -0.5f + location[2] };
    vec3 v7 = {  0.5f + location[0],  0.5f + location[1], -0.5f + location[2] };

    block.right_face = {
        v1[0], v1[1], v1[2], uv_sides[0] + uv_pad, uv_sides[1] + uv_pad,
        v7[0], v7[1], v7[2], uv_sides[0] + uw,     uv_sides[1] + vh,
        v3[0], v3[1], v3[2], uv_sides[0] + uv_pad, uv_sides[1] + vh,
        v7[0], v7[1], v7[2], uv_sides[0] + uw,     uv_sides[1] + vh,
        v1[0], v1[1], v1[2], uv_sides[0] + uv_pad, uv_sides[1] + uv_pad,
        v5[0], v5[1], v5[2], uv_sides[0] + uw,     uv_sides[1] + uv_pad
    };

    block.left_face = {
        v4[0], v4[1], v4[2], uv_sides[0] + uv_pad, uv_sides[1] + uv_pad,
        v2[0], v2[1], v2[2], uv_sides[0] + uw,     uv_sides[1] + vh,
        v6[0], v6[1], v6[2], uv_sides[0] + uv_pad, uv_sides[1] + vh,
        v2[0], v2[1], v2[2], uv_sides[0] + uw,     uv_sides[1] + vh,
        v4[0], v4[1], v4[2], uv_sides[0] + uv_pad, uv_sides[1] + uv_pad,
        v0[0], v0[1], v0[2], uv_sides[0] + uw,     uv_sides[1] + uv_pad
    };

    block.front_face = {
        v0[0], v0[1], v0[2], uv_sides[0] + uv_pad, uv_sides[1] + uv_pad,
        v3[0], v3[1], v3[2], uv_sides[0] + uw,     uv_sides[1] + vh,
        v2[0], v2[1], v2[2], uv_sides[0] + uv_pad, uv_sides[1] + vh,
        v3[0], v3[1], v3[2], uv_sides[0] + uw,     uv_sides[1] + vh,
        v0[0], v0[1], v0[2], uv_sides[0] + uv_pad, uv_sides[1] + uv_pad,
        v1[0], v1[1], v1[2], uv_sides[0] + uw,     uv_sides[1] + uv_pad
    };

    block.back_face = {
        v5[0], v5[1], v5[2], uv_sides[0] + uv_pad, uv_sides[1] + uv_pad,
        v6[0], v6[1], v6[2], uv_sides[0] + uw,     uv_sides[1] + vh,
        v7[0], v7[1], v7[2], uv_sides[0] + uv_pad, uv_sides[1] + vh,
        v6[0], v6[1], v6[2], uv_sides[0] + uw,     uv_sides[1] + vh,
        v5[0], v5[1], v5[2], uv_sides[0] + uv_pad, uv_sides[1] + uv_pad,
        v4[0], v4[1], v4[2], uv_sides[0] + uw,     uv_sides[1] + uv_pad
    };

    block.bottom_face = {
        v2[0], v2[1], v2[2], uv_bottom[0] + uv_pad, uv_bottom[1] + vh,
        v3[0], v3[1], v3[2], uv_bottom[0] + uw,     uv_bottom[1] + vh,
        v6[0], v6[1], v6[2], uv_bottom[0] + uv_pad, uv_bottom[1] + uv_pad,
        v7[0], v7[1], v7[2], uv_bottom[0] + uw,     uv_bottom[1] + uv_pad,
        v6[0], v6[1], v6[2], uv_bottom[0] + uv_pad, uv_bottom[1] + uv_pad,
        v3[0], v3[1], v3[2], uv_bottom[0] + uw,     uv_bottom[1] + vh
    };

    block.top_face = {
        v4[0], v4[1], v4[2], uv_top[0] + uv_pad, uv_top[1] + uv_pad,
        v1[0], v1[1], v1[2], uv_top[0] + uw,     uv_top[1] + vh,
        v0[0], v0[1], v0[2], uv_top[0] + uv_pad, uv_top[1] + vh,
        v1[0], v1[1], v1[2], uv_top[0] + uw,     uv_top[1] + vh,
        v4[0], v4[1], v4[2], uv_top[0] + uv_pad, uv_top[1] + uv_pad,
        v5[0], v5[1], v5[2], uv_top[0] + uw,     uv_top[1] + uv_pad
    };

    return block;
}
