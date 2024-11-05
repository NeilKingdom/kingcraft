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
    SkyBox();
    SkyBox(const cube_map_textures_t textures, const unsigned min_filter, const unsigned mag_filter);
    ~SkyBox();

    // General
    void bind() const;
    void unbind() const;

private:
    PngHndl_t *m_png_hndl;
};
