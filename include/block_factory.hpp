#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "block.hpp"

class BlockFactory
{
public:
    // Special member functions
    BlockFactory(const BlockFactory &block_factory) = delete;
    BlockFactory &operator=(const BlockFactory &block_factory) = delete;
    BlockFactory(BlockFactory &&block_factory) = delete;
    BlockFactory &operator=(BlockFactory &&block_factory) = delete;

    // General
    static BlockFactory &get_instance();
    Block make_block(const BlockType type, const uint8_t faces, const Vec3_t world_location) const;

private:
    // Member variables
    std::map<BlockType, std::optional<std::tuple<UV, UV, UV>>> uv_cache;

    // Special member functions
    BlockFactory();
    ~BlockFactory() = default;

    // General
    std::optional<std::tuple<UV, UV, UV>> get_uv_coords(const BlockType type) const;
};
