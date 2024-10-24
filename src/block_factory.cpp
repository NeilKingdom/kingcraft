#include "block_factory.hpp"

BlockFactory::BlockFactory()
{}

BlockFactory &BlockFactory::get_instance()
{
    static BlockFactory instance;
    return instance;
}

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

/*
 *              z (+up)
 * (+forward) x |
 *             \|
 *              +---y (+right)
 */
Block BlockFactory::make_block(const BlockType type, const mat4 m_trns, const Face sides) const
{
    typedef const std::array<float, 30> face_t;

    Block block = Block(type);
    auto vertices = std::vector<float>();

    const auto add_face = [&](face_t a)
    {
        for (auto i = a.begin(); i != a.end(); ++i)
        {
            vertices.push_back(*i);
        }
    };

    // UV coordinates
    float u_off = 1.0f / KCConst::TEX_SIZE;
    float v_off = 1.0f / KCConst::TEX_SIZE;

    auto uv = get_tex_by_block_type(type).value_or(
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
        v1[0], v1[1], v1[2], uv_sides[0],         uv_sides[1],
        v7[0], v7[1], v7[2], uv_sides[0] + u_off, uv_sides[1] + v_off,
        v3[0], v3[1], v3[2], uv_sides[0],         uv_sides[1] + v_off,
        v7[0], v7[1], v7[2], uv_sides[0] + u_off, uv_sides[1] + v_off,
        v1[0], v1[1], v1[2], uv_sides[0],         uv_sides[1],
        v5[0], v5[1], v5[2], uv_sides[0] + u_off, uv_sides[1]
    };

    const face_t left = {
        v4[0], v4[1], v4[2], uv_sides[0],         uv_sides[1],
        v2[0], v2[1], v2[2], uv_sides[0] + u_off, uv_sides[1] + v_off,
        v6[0], v6[1], v6[2], uv_sides[0],         uv_sides[1] + v_off,
        v2[0], v2[1], v2[2], uv_sides[0] + u_off, uv_sides[1] + v_off,
        v4[0], v4[1], v4[2], uv_sides[0],         uv_sides[1],
        v0[0], v0[1], v0[2], uv_sides[0] + u_off, uv_sides[1]
    };

    const face_t back = {
        v0[0], v0[1], v0[2], uv_sides[0],         uv_sides[1],
        v3[0], v3[1], v3[2], uv_sides[0] + u_off, uv_sides[1] + v_off,
        v2[0], v2[1], v2[2], uv_sides[0],         uv_sides[1] + v_off,
        v3[0], v3[1], v3[2], uv_sides[0] + u_off, uv_sides[1] + v_off,
        v0[0], v0[1], v0[2], uv_sides[0],         uv_sides[1],
        v1[0], v1[1], v1[2], uv_sides[0] + u_off, uv_sides[1]
    };

    const face_t front = {
        v5[0], v5[1], v5[2], uv_sides[0],         uv_sides[1],
        v6[0], v6[1], v6[2], uv_sides[0] + u_off, uv_sides[1] + v_off,
        v7[0], v7[1], v7[2], uv_sides[0],         uv_sides[1] + v_off,
        v6[0], v6[1], v6[2], uv_sides[0] + u_off, uv_sides[1] + v_off,
        v5[0], v5[1], v5[2], uv_sides[0],         uv_sides[1],
        v4[0], v4[1], v4[2], uv_sides[0] + u_off, uv_sides[1]
    };

    const face_t bottom = {
        v2[0], v2[1], v2[2], uv_bottom[0],         uv_bottom[1] + v_off,
        v3[0], v3[1], v3[2], uv_bottom[0] + u_off, uv_bottom[1] + v_off,
        v6[0], v6[1], v6[2], uv_bottom[0],         uv_bottom[1],
        v7[0], v7[1], v7[2], uv_bottom[0] + u_off, uv_bottom[1],
        v6[0], v6[1], v6[2], uv_bottom[0],         uv_bottom[1],
        v3[0], v3[1], v3[2], uv_bottom[0] + u_off, uv_bottom[1] + v_off
    };

    const face_t top = {
        v4[0], v4[1], v4[2], uv_top[0],         uv_top[1],
        v1[0], v1[1], v1[2], uv_top[0] + u_off, uv_top[1] + v_off,
        v0[0], v0[1], v0[2], uv_top[0],         uv_top[1] + v_off,
        v1[0], v1[1], v1[2], uv_top[0] + u_off, uv_top[1] + v_off,
        v4[0], v4[1], v4[2], uv_top[0],         uv_top[1],
        v5[0], v5[1], v5[2], uv_top[0] + u_off, uv_top[1]
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
