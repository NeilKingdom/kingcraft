#include "chunk_column.hpp"

ChunkColumn make_chunk_column(vec2 location)
{
    ChunkColumn chunk_col;

    GameState &game = GameState::get_instance();
    ChunkFactory &chunk_factory = ChunkFactory::get_instance();
    ssize_t chunk_size = game.chunk_size;
    ssize_t height, height_lo, height_hi;

    std::memcpy(chunk_col.location, location, sizeof(vec2));

    height_lo = KC::CHUNK_Z_LIMIT;
    height_hi = 0;

    std::vector<std::vector<uint8_t>> heights;
    heights.resize(chunk_size, std::vector<uint8_t>(chunk_size));

    for (ssize_t y = 0; y < chunk_size; ++y)
    {
        for (ssize_t x = 0; x < chunk_size; ++x)
        {
            height = heights[y][x] = game.pn.octave_perlin(
                -location[0] * chunk_size + x,
                 location[1] * chunk_size + y,
                 0.8f,
                 0.05f, 0, (KC::CHUNK_Z_LIMIT - 1)
            );

            if (height < height_lo)
            {
                height_lo = height;
            }

            if (height > height_hi)
            {
                height_hi = height;
            }
        }
    }

    //for (int i = 0; i < (height_hi / chunk_size) + 1; ++i)
    //{
    //    vec3 tmp_location = { location[0], location[1], (float)i };
    //    if (i < (height_lo / chunk_size))
    //    {
    //        chunk_col.chunk_col.push_back(chunk_factory.make_solid_chunk(tmp_location, BOTTOM));
    //    }
    //    else
    //    {
    //        chunk_col.chunk_col.push_back(chunk_factory.make_chunk(tmp_location, ALL));
    //    }
    //}

    for (int i = (height_lo / chunk_size); i < (height_hi / chunk_size) + 1; ++i)
    {
        vec3 tmp_location = { location[0], location[1], (float)i };
        chunk_col.chunk_col.push_back(chunk_factory.make_chunk(tmp_location, ALL));
    }

    return chunk_col;
}

bool ChunkColumn::operator==(const ChunkColumn &chunk_col) const
{
    return location[0] == chunk_col.location[0]
        && location[1] == chunk_col.location[1];
}

bool ChunkColumn::operator<(const ChunkColumn &chunk_col) const
{
    return (location[1] < chunk_col.location[1]) ? true :
        (location[0] < chunk_col.location[0]) ? true : false;
}
