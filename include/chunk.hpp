#pragma once

#include "common.hpp"
#include "block.hpp"
#include "game_state.hpp"

// Forward declaration
class ChunkFactory;

class Chunk
{
public:
    vec2 location;
    std::vector<std::vector<std::vector<std::unique_ptr<Block>>>> blocks;

    // Special member functions
    Chunk() : location{}
    {
        GameState &game = GameState::get_instance();

        //blocks.resize(
        //    game.chunk_size,
        //    std::vector<std::vector<std::unique_ptr<Block>>>(
        //        game.chunk_size,
        //        std::vector<std::unique_ptr<Block>>(game.chunk_size)
        //    )
        //);

        //m_block_faces.resize(
        //    game.chunk_size,
        //    std::vector<std::vector<uint8_t>>(
        //        game.chunk_size,
        //        std::vector<uint8_t>(game.chunk_size, 0)
        //    )
        //);
    }
    ~Chunk() = default;

    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;

    // TODO: General
    //void add_block(const BlockType type, const uint8_t x, const uint8_t y, const uint8_t z);
    //void remove_block(const uint8_t x, const uint8_t y, const uint8_t z);

private:
    friend ChunkFactory;
    //std::vector<std::vector<std::vector<uint8_t>>> m_block_faces;
};
