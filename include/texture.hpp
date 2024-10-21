#pragma once

#include <pixmap.h>

#include "common.hpp"

class Texture
{
public:
    unsigned id;

    // Special member functions
    Texture();
    Texture(const Pixmap_t pixmap, const unsigned min_filter, const unsigned mag_filter, const bool make_mipmap = false);
    ~Texture() = default;

    // General
    void bind() const;
    void unbind() const;
};
