#include "block_factory.hpp"

BlockFactory::BlockFactory()
{}

BlockFactory &BlockFactory::get_instance()
{
    static BlockFactory instance;
    return instance;
}

/*
 *              z (+up)
 * (+forward) x |
 *             \|
 *              +---y (+right)
 *
 *           4____5
 *          /|   /|
 *         0-+--1 |
 *         | 6__|_7
 *         |/   |/
 *         2----3
 */
Block BlockFactory::make_block(const BlockType type, const mat4 m_trns, const Face sides) const
{
    typedef const std::array<float, 30> face_t;

    Block block = Block(type);
    auto vertices = std::vector<float>();
    auto add_face = [&](face_t a)
    {
        for (auto i = a.begin(); i != a.end(); ++i)
        {
            vertices.push_back(*i);
        }
    };

    float u_off = 1.0f / 16.0f;
    float v_off = 1.0f / 16.0f;

    auto uv = get_tex_by_block_type(type).value_or(std::make_tuple(UvCoords{}, UvCoords{}, UvCoords{}));

    UvCoords uv_top    = std::get<0>(uv);
    UvCoords uv_sides  = std::get<1>(uv);
    UvCoords uv_bottom = std::get<2>(uv);

    face_t right = {
    //   X      Y      Z      U                     V
        -0.5f,  0.5f,  0.5f,  uv_sides[0],          uv_sides[1],
         0.5f,  0.5f, -0.5f,  uv_sides[0] + u_off,  uv_sides[1] + v_off,
        -0.5f,  0.5f, -0.5f,  uv_sides[0],          uv_sides[1] + v_off,
         0.5f,  0.5f, -0.5f,  uv_sides[0] + u_off,  uv_sides[1] + v_off,
        -0.5f,  0.5f,  0.5f,  uv_sides[0],          uv_sides[1],
         0.5f,  0.5f,  0.5f,  uv_sides[0] + u_off,  uv_sides[1]
    };

    face_t left = {
    //   X      Y      Z      U                     V
         0.5f, -0.5f,  0.5f,  uv_sides[0],          uv_sides[1],            // 4
        -0.5f, -0.5f, -0.5f,  uv_sides[0] + u_off,  uv_sides[1] + v_off,    // 2
         0.5f, -0.5f, -0.5f,  uv_sides[0],          uv_sides[1] + v_off,    // 6
        -0.5f, -0.5f, -0.5f,  uv_sides[0] + u_off,  uv_sides[1] + v_off,    // 2
         0.5f, -0.5f,  0.5f,  uv_sides[0],          uv_sides[1],            // 4
        -0.5f, -0.5f,  0.5f,  uv_sides[0] + u_off,  uv_sides[1]             // 0
    };

    face_t back = {
    //   X      Y      Z      U                     V
        -0.5f, -0.5f,  0.5f,  uv_sides[0],          uv_sides[1],           // 0
        -0.5f,  0.5f, -0.5f,  uv_sides[0] + u_off,  uv_sides[1] + v_off,   // 3
        -0.5f, -0.5f, -0.5f,  uv_sides[0],          uv_sides[1] + v_off,   // 2
        -0.5f,  0.5f, -0.5f,  uv_sides[0] + u_off,  uv_sides[1] + v_off,   // 3
        -0.5f, -0.5f,  0.5f,  uv_sides[0],          uv_sides[1],           // 0
        -0.5f,  0.5f,  0.5f,  uv_sides[0] + u_off,  uv_sides[1]            // 1
    };

    face_t front = {
    //   X      Y      Z      U                     V
         0.5f,  0.5f,  0.5f,  uv_sides[0],          uv_sides[1],           // 5
         0.5f, -0.5f, -0.5f,  uv_sides[0] + u_off,  uv_sides[1] + v_off,   // 6
         0.5f,  0.5f, -0.5f,  uv_sides[0],          uv_sides[1] + v_off,   // 7
         0.5f, -0.5f, -0.5f,  uv_sides[0] + u_off,  uv_sides[1] + v_off,   // 6
         0.5f,  0.5f,  0.5f,  uv_sides[0],          uv_sides[1],           // 5
         0.5f, -0.5f,  0.5f,  uv_sides[0] + u_off,  uv_sides[1]            // 4
    };

    face_t bottom = {
    //   X      Y      Z      U                     V
        -0.5f, -0.5f, -0.5f,  uv_bottom[0],          uv_bottom[1] + v_off,  // 2
        -0.5f,  0.5f, -0.5f,  uv_bottom[0] + u_off,  uv_bottom[1] + v_off,  // 3
         0.5f, -0.5f, -0.5f,  uv_bottom[0],          uv_bottom[1],          // 6
         0.5f,  0.5f, -0.5f,  uv_bottom[0] + u_off,  uv_bottom[1],          // 7
         0.5f, -0.5f, -0.5f,  uv_bottom[0],          uv_bottom[1],          // 6
        -0.5f,  0.5f, -0.5f,  uv_bottom[0] + u_off,  uv_bottom[1] + v_off   // 3
    };

    face_t top = {
    //   X      Y      Z      U                   V
         0.5f, -0.5f,  0.5f,  uv_top[0],          uv_top[1],            // 4
        -0.5f,  0.5f,  0.5f,  uv_top[0] + u_off,  uv_top[1] + v_off,    // 1
        -0.5f, -0.5f,  0.5f,  uv_top[0],          uv_top[1] + v_off,    // 0
        -0.5f,  0.5f,  0.5f,  uv_top[0] + u_off,  uv_top[1] + v_off,    // 1
         0.5f, -0.5f,  0.5f,  uv_top[0],          uv_top[1],            // 4
         0.5f,  0.5f,  0.5f,  uv_top[0] + u_off,  uv_top[1]             // 5
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

    // TODO: Translate cube

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
