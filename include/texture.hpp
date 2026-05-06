#pragma once

#include "common.hpp"
#include <png_parser.h>

class Texture
{
public:
    // Member variables
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
    Texture(const Texture &texture) = default;
    Texture &operator=(const Texture &texture) = default;
    Texture(Texture &&texture) = default;
    Texture &operator=(Texture &&texture) = default;

    // General
    void bind() const;
    void unbind() const;
};
