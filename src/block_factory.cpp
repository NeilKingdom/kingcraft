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

            uv_top[0] = uv_sides[0] = uv_bottom[0] = tx_offset / (float)KC::ATLAS_TEX_SIZE;
            uv_top[1] = uv_sides[1] = uv_bottom[1] = ty_offset;
            break;
        case BlockType::GRASS:
            ty_offset = 0.0f;

            // Top
            tx_offset = 0.0f;
            uv_top[0] = tx_offset / (float)KC::ATLAS_TEX_SIZE;
            uv_top[1] = ty_offset;

            // Sides
            tx_offset = 1.0f;
            uv_sides[0] = tx_offset / (float)KC::ATLAS_TEX_SIZE;
            uv_sides[1] = ty_offset;

            // Bottom
            tx_offset = 2.0f;
            uv_bottom[0] = tx_offset / (float)KC::ATLAS_TEX_SIZE;
            uv_bottom[1] = ty_offset;
            break;
        default:
            return std::nullopt;
    }

    return std::make_tuple(uv_top, uv_sides, uv_bottom);
}

/**
 * @brief Creates a single block based on a set of given parameters.
 * @since 16-10-2024
 * @param[in] type The block type of the block being created e.g. dirt, grass, etc.
 * @param[in] m_block_trns A 4x4 matrix which determines the location of the block relative to the parent chunk
 * @param[in] sides A mask which determines which sides of the cube will be rendered
 * @returns A Block object with the specified attributes
 */
std::shared_ptr<Block> BlockFactory::make_block(
    const BlockType type,
    const vec3 location,
    const uint8_t sides
)
{
    typedef const std::array<float, 30> face_t;
    auto block = std::make_shared<Block>(Block(type));

    if (sides == 0 || block->type == BlockType::AIR)
    {
        return std::make_shared<Block>(Block(BlockType::AIR));
    }

    auto vertices = std::vector<float>();
    vertices.reserve(sizeof(face_t) / sizeof(float) * KC::CUBE_FACES);

    const auto add_face = [&](face_t f)
    {
        for (auto i = f.begin(); i != f.end(); ++i)
        {
            vertices.emplace_back(*i);
        }
    };

    // UV coordinates
    constexpr float uv_pad = 0.005f;
    constexpr float uw = (1.0f / KC::ATLAS_TEX_SIZE) - uv_pad;
    constexpr float vh = (1.0f / KC::ATLAS_TEX_SIZE) - uv_pad;

    auto uv = get_uv_coords(type).value_or(
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

    const face_t right = {
        v1[0], v1[1], v1[2], uv_sides[0] + uv_pad, uv_sides[1] + uv_pad,
        v7[0], v7[1], v7[2], uv_sides[0] + uw,     uv_sides[1] + vh,
        v3[0], v3[1], v3[2], uv_sides[0] + uv_pad, uv_sides[1] + vh,
        v7[0], v7[1], v7[2], uv_sides[0] + uw,     uv_sides[1] + vh,
        v1[0], v1[1], v1[2], uv_sides[0] + uv_pad, uv_sides[1] + uv_pad,
        v5[0], v5[1], v5[2], uv_sides[0] + uw,     uv_sides[1] + uv_pad
    };

    const face_t left = {
        v4[0], v4[1], v4[2], uv_sides[0] + uv_pad, uv_sides[1] + uv_pad,
        v2[0], v2[1], v2[2], uv_sides[0] + uw,     uv_sides[1] + vh,
        v6[0], v6[1], v6[2], uv_sides[0] + uv_pad, uv_sides[1] + vh,
        v2[0], v2[1], v2[2], uv_sides[0] + uw,     uv_sides[1] + vh,
        v4[0], v4[1], v4[2], uv_sides[0] + uv_pad, uv_sides[1] + uv_pad,
        v0[0], v0[1], v0[2], uv_sides[0] + uw,     uv_sides[1] + uv_pad
    };

    const face_t front = {
        v0[0], v0[1], v0[2], uv_sides[0] + uv_pad, uv_sides[1] + uv_pad,
        v3[0], v3[1], v3[2], uv_sides[0] + uw,     uv_sides[1] + vh,
        v2[0], v2[1], v2[2], uv_sides[0] + uv_pad, uv_sides[1] + vh,
        v3[0], v3[1], v3[2], uv_sides[0] + uw,     uv_sides[1] + vh,
        v0[0], v0[1], v0[2], uv_sides[0] + uv_pad, uv_sides[1] + uv_pad,
        v1[0], v1[1], v1[2], uv_sides[0] + uw,     uv_sides[1] + uv_pad
    };

    const face_t back = {
        v5[0], v5[1], v5[2], uv_sides[0] + uv_pad, uv_sides[1] + uv_pad,
        v6[0], v6[1], v6[2], uv_sides[0] + uw,     uv_sides[1] + vh,
        v7[0], v7[1], v7[2], uv_sides[0] + uv_pad, uv_sides[1] + vh,
        v6[0], v6[1], v6[2], uv_sides[0] + uw,     uv_sides[1] + vh,
        v5[0], v5[1], v5[2], uv_sides[0] + uv_pad, uv_sides[1] + uv_pad,
        v4[0], v4[1], v4[2], uv_sides[0] + uw,     uv_sides[1] + uv_pad
    };

    const face_t bottom = {
        v2[0], v2[1], v2[2], uv_bottom[0] + uv_pad, uv_bottom[1] + vh,
        v3[0], v3[1], v3[2], uv_bottom[0] + uw,     uv_bottom[1] + vh,
        v6[0], v6[1], v6[2], uv_bottom[0] + uv_pad, uv_bottom[1] + uv_pad,
        v7[0], v7[1], v7[2], uv_bottom[0] + uw,     uv_bottom[1] + uv_pad,
        v6[0], v6[1], v6[2], uv_bottom[0] + uv_pad, uv_bottom[1] + uv_pad,
        v3[0], v3[1], v3[2], uv_bottom[0] + uw,     uv_bottom[1] + vh
    };

    const face_t top = {
        v4[0], v4[1], v4[2], uv_top[0] + uv_pad, uv_top[1] + uv_pad,
        v1[0], v1[1], v1[2], uv_top[0] + uw,     uv_top[1] + vh,
        v0[0], v0[1], v0[2], uv_top[0] + uv_pad, uv_top[1] + vh,
        v1[0], v1[1], v1[2], uv_top[0] + uw,     uv_top[1] + vh,
        v4[0], v4[1], v4[2], uv_top[0] + uv_pad, uv_top[1] + uv_pad,
        v5[0], v5[1], v5[2], uv_top[0] + uw,     uv_top[1] + uv_pad
    };

    if (IS_BIT_SET(sides, RIGHT))
    {
        add_face(right);
        SET_BIT(block->faces, RIGHT);
    }
    if (IS_BIT_SET(sides, LEFT))
    {
        add_face(left);
        SET_BIT(block->faces, LEFT);
    }
    if (IS_BIT_SET(sides, BACK))
    {
        add_face(back);
        SET_BIT(block->faces, BACK);
    }
    if (IS_BIT_SET(sides, FRONT))
    {
        add_face(front);
        SET_BIT(block->faces, FRONT);
    }
    if (IS_BIT_SET(sides, BOTTOM))
    {
        add_face(bottom);
        SET_BIT(block->faces, BOTTOM);
    }
    if (IS_BIT_SET(sides, TOP))
    {
        add_face(top);
        SET_BIT(block->faces, TOP);
    }

    vertices.shrink_to_fit();
    block->mesh.vertices = vertices.size();

    // Create vertex attribute array and vertex buffer object
    glGenVertexArrays(1, &block->mesh.vao);
    glGenBuffers(1, &block->mesh.vbo);

    glBindVertexArray(block->mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, block->mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // TODO: Color attribute

    // Texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind VAO and VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return block;
}
