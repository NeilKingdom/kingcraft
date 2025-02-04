#include "chunk.hpp"

Chunk::~Chunk()
{
    if (glIsBuffer(mesh.vbo))
    {
        glDeleteBuffers(1, &mesh.vbo);
    }
    if (glIsVertexArray(mesh.vao))
    {
        glDeleteVertexArrays(1, &mesh.vao);
    }
}

bool Chunk::operator==(const Chunk &chunk) const
{
    return this->location[0] == chunk.location[0]
        && this->location[1] == chunk.location[1]
        && this->location[2] == chunk.location[2];
}

void Chunk::add_block(const BlockType type, const vec3 location)
{

}

void Chunk::remove_block(const vec3 location)
{
    blocks[location[2]][location[1]][location[0]].type = BlockType::AIR;
    // TODO: Regenerate neighboring blocks
    mesh.vertices.clear();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    if (glIsBuffer(mesh.vbo))
    {
        glDeleteBuffers(1, &mesh.vbo);
    }
    if (glIsVertexArray(mesh.vao))
    {
        glDeleteVertexArrays(1, &mesh.vao);
    }

    squash_block_meshes();
}

void Chunk::squash_block_meshes()
{
    GameState &game = GameState::get_instance();
    ssize_t chunk_size = game.chunk_size;
    auto &chunk_mesh = mesh.vertices;

    for (ssize_t z = 0; z < chunk_size; ++z)
    {
        for (ssize_t y = 0; y < chunk_size; ++y)
        {
            for (ssize_t x = 0; x < chunk_size; ++x)
             {
                Block block = blocks[z][y][x];
                if (block.type != BlockType::AIR && block.faces != 0)
                {
                    if (IS_BIT_SET(block.faces, BOTTOM))
                    {
                        chunk_mesh.insert(chunk_mesh.end(), block.bottom_face.begin(), block.bottom_face.end());
                    }
                    if (IS_BIT_SET(block.faces, TOP))
                    {
                        chunk_mesh.insert(chunk_mesh.end(), block.top_face.begin(), block.top_face.end());
                    }
                    if (IS_BIT_SET(block.faces, RIGHT))
                    {
                        chunk_mesh.insert(chunk_mesh.end(), block.right_face.begin(), block.right_face.end());
                    }
                    if (IS_BIT_SET(block.faces, LEFT))
                    {
                        chunk_mesh.insert(chunk_mesh.end(), block.left_face.begin(), block.left_face.end());
                    }
                    if (IS_BIT_SET(block.faces, FRONT))
                    {
                        chunk_mesh.insert(chunk_mesh.end(), block.front_face.begin(), block.front_face.end());
                    }
                    if (IS_BIT_SET(block.faces, BACK))
                    {
                        chunk_mesh.insert(chunk_mesh.end(), block.back_face.begin(), block.back_face.end());
                    }
                }
             }
        }
    }

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, chunk_mesh.size() * sizeof(float), chunk_mesh.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // TODO: Color attribute

    // Texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
