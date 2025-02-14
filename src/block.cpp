/**
 * @file block.cpp
 * @author Neil Kingdom
 * @since 16-10-2024
 * @version 1.0
 * @brief A structure which represents a single voxel/block.
 */

#include "block.hpp"

/**
 * @brief Default constructor for Block struct.
 * @since 13-02-2025
 */
Block::Block() :
    type(BlockType::AIR), faces(0)
{}

/**
 * @brief Parameterized constructor for creating a block of type __type__.
 * @param[in] type The type of block being generated
 */
Block::Block(const BlockType type) :
    type(type), faces(ALL)
{}
