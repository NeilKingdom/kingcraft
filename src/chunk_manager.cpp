#include "chunk_manager.hpp"

ChunkManager::ChunkManager() :
    chunks{},
    chunk_cache{},
    chunk_col_coords{}
{}

// TODO: Could return bool?
// TODO: Add flag to specify whether blocks should be replaceable
void ChunkManager::add_block(std::shared_ptr<Chunk> &chunk, const BlockType type, const vec3 location)
{
    BlockFactory block_factory;
    GameState &game = GameState::get_instance();
    ssize_t chunk_size = game.chunk_size;

    if (location[0] < 0 || location[0] > (chunk_size - 1) ||
        location[1] < 0 || location[1] > (chunk_size - 1) ||
        location[2] < 0 || location[2] > (chunk_size - 1) ||
        chunk->blocks[location[2]][location[1]][location[0]].type != BlockType::AIR)
    {
        return;
    }

    Block &block = chunk->blocks[location[2]][location[1]][location[0]];
    if (block.type == BlockType::AIR)
    {
        vec3 real_location = {
            -(chunk->location[0] * chunk_size) + location[0],
             (chunk->location[1] * chunk_size) + location[1],
             (chunk->location[2] * chunk_size) + location[2]
        };
        block = block_factory.make_block(type, real_location, ALL);
    }

    block.faces = ALL;
    block.type = type;

    // Remove block faces

    //Block &behind   = chunk->blocks[location[2]][location[1]][location[0] + 1];
    //Block &in_front = chunk->blocks[location[2]][location[1]][location[0] - 1];
    //Block &to_right = chunk->blocks[location[2]][location[1] + 1][location[0]];
    //Block &to_left  = chunk->blocks[location[2]][location[1] - 1][location[0]];
    //Block &above    = chunk->blocks[location[2] + 1][location[1]][location[0]];
    //Block &below    = chunk->blocks[location[2] - 1][location[1]][location[0]];

    // TODO: Nefarious bug in here!
    if (block.type != BlockType::LEAVES)
    {
        if (
            location[0] > 0 &&
            chunk->blocks[location[2]][location[1]][location[0] - 1].type != BlockType::AIR &&
            chunk->blocks[location[2]][location[1]][location[0] - 1].type != BlockType::LEAVES
        )
        {
            UNSET_BIT(chunk->blocks[location[2]][location[1]][location[0] - 1].faces, BACK);
            UNSET_BIT(block.faces, FRONT);
        }
        if (
            location[0] < (chunk_size - 1) &&
            chunk->blocks[location[2]][location[1]][location[0] + 1].type != BlockType::AIR &&
            chunk->blocks[location[2]][location[1]][location[0] + 1].type != BlockType::LEAVES
        )
        {
            UNSET_BIT(chunk->blocks[location[2]][location[1]][location[0] + 1].faces, FRONT);
            UNSET_BIT(block.faces, BACK);
        }
        if (location[1] > 0 &&
            chunk->blocks[location[2]][location[1] - 1][location[0]].type != BlockType::AIR &&
            chunk->blocks[location[2]][location[1] - 1][location[0]].type != BlockType::LEAVES
        )
        {
            UNSET_BIT(chunk->blocks[location[2]][location[1] - 1][location[0]].faces, RIGHT);
            UNSET_BIT(block.faces, LEFT);
        }
        if (location[1] < (chunk_size - 1) &&
            chunk->blocks[location[2]][location[1] + 1][location[0]].type != BlockType::AIR &&
            chunk->blocks[location[2]][location[1] + 1][location[0]].type != BlockType::LEAVES
        )
        {
            UNSET_BIT(chunk->blocks[location[2]][location[1] + 1][location[0]].faces, LEFT);
            UNSET_BIT(block.faces, RIGHT);
        }
        if (location[2] > 0 &&
            chunk->blocks[location[2] - 1][location[1]][location[0]].type != BlockType::AIR &&
            chunk->blocks[location[2] - 1][location[1]][location[0]].type != BlockType::LEAVES
        )
        {
            UNSET_BIT(chunk->blocks[location[2] - 1][location[1]][location[0]].faces, TOP);
            UNSET_BIT(block.faces, BOTTOM);
        }
        if (location[2] < (chunk_size - 1) &&
            chunk->blocks[location[2] + 1][location[1]][location[0]].type != BlockType::AIR &&
            chunk->blocks[location[2] + 1][location[1]][location[0]].type != BlockType::LEAVES
        )
        {
            UNSET_BIT(chunk->blocks[location[2] + 1][location[1]][location[0]].faces, BOTTOM);
            UNSET_BIT(block.faces, TOP);
        }
    }

    // TODO: Add chunk to cache if not already present
}

void ChunkManager::remove_block(std::shared_ptr<Chunk> &chunk, const vec3 location)
{
    chunk->blocks[location[2]][location[1]][location[0]].type = BlockType::AIR;

    // TODO: Regenerate neighboring block faces
}

std::optional<std::shared_ptr<Chunk>> ChunkManager::get_chunk_at_location_bi(const vec3 location) const
{
    GameState &game = GameState::get_instance();
    ssize_t chunk_size = game.chunk_size;

    vec3 chunk_index = {
        (float)((ssize_t)location[0] % chunk_size),
        (float)((ssize_t)location[1] % chunk_size),
        (float)((ssize_t)location[2] % chunk_size),
    };

    return get_chunk_at_location_ci(chunk_index);
}

std::optional<std::shared_ptr<Chunk>> ChunkManager::get_chunk_at_location_ci(const vec3 location) const
{
    Chunk needle;
    std::memcpy(needle.location, location, sizeof(vec3));
    auto found = std::find_if(
        chunks.begin(),
        chunks.end(),
        [&](const std::shared_ptr<Chunk> &chunk)
        {
            return needle == *chunk;
        }
    );
    return (found != chunks.end()) ? std::make_optional(*found) : std::nullopt;
}
