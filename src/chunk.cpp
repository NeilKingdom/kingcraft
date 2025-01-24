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

    flatten_block_data();
}

void Chunk::flatten_block_data()
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
                    chunk_mesh.insert(chunk_mesh.begin(), block.vertices.begin(), block.vertices.end());
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
