#pragma once

#include "common.hpp"
#include "texture.hpp"
#include "mesh.hpp"

class SkyBox
{
public:
    ID id;
    Mesh mesh;

    // Special member functions
    SkyBox();
    SkyBox(std::array<std::filesystem::path, 6> tex_paths, const unsigned min_filter, const unsigned mag_filter);
    ~SkyBox();

    // General
    void bind() const;
    void unbind() const;

private:
    PngHndl_t *m_png_hndl;
};
