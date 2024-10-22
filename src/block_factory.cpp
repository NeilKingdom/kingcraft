#include "block_factory.hpp"

BlockFactory::BlockFactory()
{}

BlockFactory &BlockFactory::get_instance()
{
    static BlockFactory instance;
    return instance;
}

Block BlockFactory::make_block(const BlockType type, const mat4 m_trns, const uint8_t sides) const
{
    Block block = Block(type);

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

    // Face to render: NIL NIL TOP BTM FRT BCK LFT RHT
    // Bit position:   7   6   5   4   3   2   1   0

    // Create base cube at (0, 0, 0)
    float vertices[] = {
    //   Positions            Texture coords
    //   X      Y      Z      U      V
    // Front
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, // 0
        -0.5f,  0.5f, -0.5f,  1.0f,  1.0f, // 3
        -0.5f, -0.5f, -0.5f,  0.0f,  1.0f, // 2
        -0.5f,  0.5f, -0.5f,  1.0f,  1.0f, // 3
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, // 0
        -0.5f,  0.5f,  0.5f,  1.0f,  0.0f, // 1

    // Top
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, // 4
        -0.5f,  0.5f,  0.5f,  1.0f,  1.0f, // 1
        -0.5f, -0.5f,  0.5f,  0.0f,  1.0f, // 0
        -0.5f,  0.5f,  0.5f,  1.0f,  1.0f, // 1
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, // 4
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f, // 5

    // Back
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, // 5
         0.5f, -0.5f, -0.5f,  1.0f,  1.0f, // 6
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f, // 7
         0.5f, -0.5f, -0.5f,  1.0f,  1.0f, // 6
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, // 5
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f, // 4

    // Bottom
        -0.5f, -0.5f, -0.5f,  0.0f,  1.0f, // 2
        -0.5f,  0.5f, -0.5f,  1.0f,  1.0f, // 3
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, // 6
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f, // 7
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, // 6
        -0.5f,  0.5f, -0.5f,  1.0f,  1.0f, // 3

    // Right
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, // 1
         0.5f,  0.5f, -0.5f,  1.0f,  1.0f, // 7
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f, // 3
         0.5f,  0.5f, -0.5f,  1.0f,  1.0f, // 7
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, // 1
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f, // 5

    // Left
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, // 4
        -0.5f, -0.5f, -0.5f,  1.0f,  1.0f, // 2
         0.5f, -0.5f, -0.5f,  0.0f,  1.0f, // 6
        -0.5f, -0.5f, -0.5f,  1.0f,  1.0f, // 2
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, // 4
        -0.5f, -0.5f,  0.5f,  1.0f,  0.0f, // 0
    };

    block.mesh.vertices = 36;

    // Create vertex attribute array and vertex buffer object
    glGenVertexArrays(1, &block.mesh.vao);
    glGenBuffers(1, &block.mesh.vbo);

    glBindVertexArray(block.mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, block.mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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
