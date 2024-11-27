#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "block.hpp"

typedef std::array<float, 2> UvCoords;

class BlockFactory
{
public:
    // Special member functions
    BlockFactory(const BlockFactory&) = delete;
    BlockFactory &operator=(const BlockFactory&) = delete;
    ~BlockFactory() = default;

    // General
    static BlockFactory &get_instance();
    void init();
    std::unique_ptr<Block> make_block(const BlockType type, const vec3 location, const uint8_t sides);

private:
    // Special member functions
    BlockFactory() = default;

    // General
    std::optional<std::tuple<UvCoords, UvCoords, UvCoords>> get_uv_coords(const BlockType type) const;
};
