#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "texture.hpp"
#include "mesh.hpp"

typedef std::array<std::filesystem::path, KC::CUBE_FACES> cube_map_textures_t;

class SkyBox
{
public:
    ID id;
    Mesh mesh;

    // Special member functions
    SkyBox() = delete;
    SkyBox(const cube_map_textures_t textures, const unsigned min_filter, const unsigned mag_filter, const bool make_mipmap = false);
    ~SkyBox();

private:
    PngHndl_t *m_png_hndl;
};
