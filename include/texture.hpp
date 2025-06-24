#pragma once

#include <png_parser.h>
#include "common.hpp"

class Texture
{
public:
    ID id;

    // Special member functions
    Texture() = delete;
    Texture(
        const std::filesystem::path path,
        const unsigned min_filter,
        const unsigned mag_filter,
        const bool make_mipmap = false
    );
    ~Texture();

    // General
    void bind() const;
    void unbind() const;
};
