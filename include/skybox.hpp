#pragma once

#include "common.hpp"
#include "constants.hpp"
#include "texture.hpp"
#include "mesh.hpp"

class SkyBox
{
public:
    ID id;
    Mesh<VPos> mesh;

    // Special member functions
    SkyBox() = delete;
    SkyBox(
        const std::array<std::filesystem::path, KC::CUBE_FACES> tex_paths,
        const unsigned min_filter,
        const unsigned mag_filter,
        const bool make_mipmap = false
    );
    ~SkyBox();
};
