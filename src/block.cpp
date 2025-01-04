#include "block.hpp"

Block::Block() : type(BlockType::AIR)
{}

Block::Block(BlockType type) : type(type)
{}

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
{}
