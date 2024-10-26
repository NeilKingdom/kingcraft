/**
 * @file block_factory.cpp
 * @author Neil Kingdom
 * @since 16-10-2024
 * @version 1.0
 * @brief A singleton class which constructs a Block object.
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
 * @brief Initializes the texture used for commmon blocks.
 * @warning This _must_ be invoked after the OpenGL context has been setup.
 * @since 26-10-2024
 */
void BlockFactory::init()
{
    m_block_tex = Texture(tex_atlas_path, GL_NEAREST, GL_NEAREST);
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
    std::optional<std::tuple<UvCoords, UvCoords, UvCoords>> uv_coords;

    switch (type)
    {
        case BlockType::DIRT:
            ty_offset = 0.0f;
            tx_offset = 2.0f;

            uv_top[0] = uv_sides[0] = uv_bottom[0] = tx_offset / (float)KCConst::ATLAS_TEX_SIZE;
            uv_top[1] = uv_sides[1] = uv_bottom[1] = ty_offset;
            break;
        case BlockType::GRASS:
            ty_offset = 0.0f;

            // Top
            tx_offset = 0.0f;
            uv_top[0] = tx_offset / (float)KCConst::ATLAS_TEX_SIZE;
            uv_top[1] = ty_offset;

            // Sides
            tx_offset = 1.0f;
            uv_sides[0] = tx_offset / (float)KCConst::ATLAS_TEX_SIZE;
            uv_sides[1] = ty_offset;

            // Bottom
            tx_offset = 2.0f;
            uv_bottom[0] = tx_offset / (float)KCConst::ATLAS_TEX_SIZE;
            uv_bottom[1] = ty_offset;

            uv_coords = std::make_tuple(uv_top, uv_sides, uv_bottom);
            break;
        default:
            uv_coords = std::nullopt;
            break;
    }

    return uv_coords;
}

/**
 * @brief Creates a single block based on a set of given parameters.
 * @since 16-10-2024
 * @param[in] type The block type of the block being created e.g. dirt, grass, etc.
 * @param[in] m_trns A 4x4 matrix which determines the location of the block relative to the world origin
 * @param[in] sides A mask which determines which sides of the cube will be rendered
 * @returns A Block object with the specified attributes
 */
Block BlockFactory::make_block(
    const BlockType type,
    const mat4 m_trns,
    const uint8_t sides
)
{
    typedef const std::array<float, 30> face_t;

    Block block(type);
    auto vertices = std::vector<float>();

    const auto add_face = [&](face_t a)
    {
        for (auto i = a.begin(); i != a.end(); ++i)
        {
            vertices.push_back(*i);
        }
    };

    // UV coordinates
    float uw = 1.0f / KCConst::ATLAS_TEX_SIZE;
    float vh = 1.0f / KCConst::ATLAS_TEX_SIZE;

    auto uv = get_uv_coords(type).value_or(
        std::make_tuple(
            UvCoords{ 0.0f, 0.0f },
            UvCoords{ 0.0f, 0.0f },
            UvCoords{ 0.0f, 0.0f }
        )
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
    vec4 v0 = { -0.5f, -0.5f,  0.5f, 1.0f };
    vec4 v1 = { -0.5f,  0.5f,  0.5f, 1.0f };
    vec4 v2 = { -0.5f, -0.5f, -0.5f, 1.0f };
    vec4 v3 = { -0.5f,  0.5f, -0.5f, 1.0f };
    vec4 v4 = {  0.5f, -0.5f,  0.5f, 1.0f };
    vec4 v5 = {  0.5f,  0.5f,  0.5f, 1.0f };
    vec4 v6 = {  0.5f, -0.5f, -0.5f, 1.0f };
    vec4 v7 = {  0.5f,  0.5f, -0.5f, 1.0f };

    // Translate cube
    vec4 copy = { 1.0f, 1.0f, 1.0f, 1.0f };

    // TODO: Remove memcpys once liblac is fixed
    lac_multiply_vec4_mat4(&copy, v0, m_trns);
    std::memcpy(v0, copy, sizeof(v0));
    lac_multiply_vec4_mat4(&copy, v1, m_trns);
    std::memcpy(v1, copy, sizeof(v1));
    lac_multiply_vec4_mat4(&copy, v2, m_trns);
    std::memcpy(v2, copy, sizeof(v2));
    lac_multiply_vec4_mat4(&copy, v3, m_trns);
    std::memcpy(v3, copy, sizeof(v3));
    lac_multiply_vec4_mat4(&copy, v4, m_trns);
    std::memcpy(v4, copy, sizeof(v4));
    lac_multiply_vec4_mat4(&copy, v5, m_trns);
    std::memcpy(v5, copy, sizeof(v5));
    lac_multiply_vec4_mat4(&copy, v6, m_trns);
    std::memcpy(v6, copy, sizeof(v6));
    lac_multiply_vec4_mat4(&copy, v7, m_trns);
    std::memcpy(v7, copy, sizeof(v7));

    const face_t right = {
        v1[0], v1[1], v1[2], uv_sides[0],      uv_sides[1],
        v7[0], v7[1], v7[2], uv_sides[0] + uw, uv_sides[1] + vh,
        v3[0], v3[1], v3[2], uv_sides[0],      uv_sides[1] + vh,
        v7[0], v7[1], v7[2], uv_sides[0] + uw, uv_sides[1] + vh,
        v1[0], v1[1], v1[2], uv_sides[0],      uv_sides[1],
        v5[0], v5[1], v5[2], uv_sides[0] + uw, uv_sides[1]
    };

    const face_t left = {
        v4[0], v4[1], v4[2], uv_sides[0],      uv_sides[1],
        v2[0], v2[1], v2[2], uv_sides[0] + uw, uv_sides[1] + vh,
        v6[0], v6[1], v6[2], uv_sides[0],      uv_sides[1] + vh,
        v2[0], v2[1], v2[2], uv_sides[0] + uw, uv_sides[1] + vh,
        v4[0], v4[1], v4[2], uv_sides[0],      uv_sides[1],
        v0[0], v0[1], v0[2], uv_sides[0] + uw, uv_sides[1]
    };

    const face_t back = {
        v0[0], v0[1], v0[2], uv_sides[0],      uv_sides[1],
        v3[0], v3[1], v3[2], uv_sides[0] + uw, uv_sides[1] + vh,
        v2[0], v2[1], v2[2], uv_sides[0],      uv_sides[1] + vh,
        v3[0], v3[1], v3[2], uv_sides[0] + uw, uv_sides[1] + vh,
        v0[0], v0[1], v0[2], uv_sides[0],      uv_sides[1],
        v1[0], v1[1], v1[2], uv_sides[0] + uw, uv_sides[1]
    };

    const face_t front = {
        v5[0], v5[1], v5[2], uv_sides[0],      uv_sides[1],
        v6[0], v6[1], v6[2], uv_sides[0] + uw, uv_sides[1] + vh,
        v7[0], v7[1], v7[2], uv_sides[0],      uv_sides[1] + vh,
        v6[0], v6[1], v6[2], uv_sides[0] + uw, uv_sides[1] + vh,
        v5[0], v5[1], v5[2], uv_sides[0],      uv_sides[1],
        v4[0], v4[1], v4[2], uv_sides[0] + uw, uv_sides[1]
    };

    const face_t bottom = {
        v2[0], v2[1], v2[2], uv_bottom[0],      uv_bottom[1] + vh,
        v3[0], v3[1], v3[2], uv_bottom[0] + uw, uv_bottom[1] + vh,
        v6[0], v6[1], v6[2], uv_bottom[0],      uv_bottom[1],
        v7[0], v7[1], v7[2], uv_bottom[0] + uw, uv_bottom[1],
        v6[0], v6[1], v6[2], uv_bottom[0],      uv_bottom[1],
        v3[0], v3[1], v3[2], uv_bottom[0] + uw, uv_bottom[1] + vh
    };

    const face_t top = {
        v4[0], v4[1], v4[2], uv_top[0],      uv_top[1],
        v1[0], v1[1], v1[2], uv_top[0] + uw, uv_top[1] + vh,
        v0[0], v0[1], v0[2], uv_top[0],      uv_top[1] + vh,
        v1[0], v1[1], v1[2], uv_top[0] + uw, uv_top[1] + vh,
        v4[0], v4[1], v4[2], uv_top[0],      uv_top[1],
        v5[0], v5[1], v5[2], uv_top[0] + uw, uv_top[1]
    };

    if ((sides & RIGHT) == RIGHT)
    {
        add_face(right);
    }
    if ((sides & LEFT) == LEFT)
    {
        add_face(left);
    }
    if ((sides & BACK) == BACK)
    {
        add_face(back);
    }
    if ((sides & FRONT) == FRONT)
    {
        add_face(front);
    }
    if ((sides & BOTTOM) == BOTTOM)
    {
        add_face(bottom);
    }
    if ((sides & TOP) == TOP)
    {
        add_face(top);
    }

    block.mesh.vertices = vertices.size();
    block.mesh.texture = m_block_tex;

    // Create vertex attribute array and vertex buffer object
    glGenVertexArrays(1, &block.mesh.vao);
    glGenBuffers(1, &block.mesh.vbo);

    glBindVertexArray(block.mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, block.mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind VAO
    glBindVertexArray(0);

    return block;
}
