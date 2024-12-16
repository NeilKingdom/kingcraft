#include "block.hpp"

Block::Block(BlockType type) :
    type(type), faces(0), mesh{}
{}

Block::~Block()
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

void Block::add_face(uint8_t face)
{
    assert(
        face == FRONT ||
        face == BACK  ||
        face == LEFT  ||
        face == RIGHT ||
        face == TOP   ||
        face == BOTTOM
    );
    SET_BIT(this->faces, face);
    modify_face();
}

void Block::remove_face(uint8_t face)
{
    assert(
        face == FRONT ||
        face == BACK  ||
        face == LEFT  ||
        face == RIGHT ||
        face == TOP   ||
        face == BOTTOM
    );
    UNSET_BIT(this->faces, face);
    modify_face();
}

void Block::modify_face()
{
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
