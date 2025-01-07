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
    return location[0] == chunk.location[0] && location[1] == chunk.location[1];
}

bool Chunk::operator<(const Chunk &chunk) const
{
    return (location[1] < chunk.location[1]) ? true : (location[0] < chunk.location[0]) ? true : false;
}

void Chunk::flatten_block_data()
{
    GameState &game = GameState::get_instance();
    ssize_t chunk_size = game.chunk_size;
    auto &aggregate = mesh.vertices;

    for (ssize_t z = 0; z < chunk_size; ++z)
    {
        for (ssize_t y = 0; y < chunk_size; ++y)
        {
            for (ssize_t x = 0; x < chunk_size; ++x)
             {
                Block block = blocks[z][y][x];
                if (block.type != BlockType::AIR && block.faces != 0)
                {
                    aggregate.insert(aggregate.begin(), block.vertices.begin(), block.vertices.end());
                }
             }
        }
    }

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, aggregate.size() * sizeof(float), aggregate.data(), GL_STATIC_DRAW);

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
