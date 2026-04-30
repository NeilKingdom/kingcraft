#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "block.hpp"

class BlockFactory
{
public:
    // Special member functions
    BlockFactory(const BlockFactory &chunk_factory) = delete;
    BlockFactory &operator=(const BlockFactory &chunk_factory) = delete;
    ~BlockFactory() = default;

    // General
    static BlockFactory &get_instance();

    Block make_block(const BlockType type, const uint8_t faces, const Vec3_t world_location) const;

private:
    std::map<BlockType, std::optional<std::tuple<UV, UV, UV>>> uv_cache;

    // Special member functions
    BlockFactory();

    // General
    std::optional<std::tuple<UV, UV, UV>> get_uv_coords(const BlockType type) const;
};
