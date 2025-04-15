#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "texture.hpp"
#include "mesh.hpp"

class SkyBox
{
public:
    ID id;
    Mesh<SkyboxVertex> mesh;

    // Special member functions
    SkyBox() = delete;
    SkyBox(const std::array<std::filesystem::path, 6> tex_paths, const unsigned min_filter, const unsigned mag_filter, const bool make_mipmap = false);
    ~SkyBox();
};
