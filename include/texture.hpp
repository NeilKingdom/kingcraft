#pragma once

#include <png_parser.h>

#include "common.hpp"

class Texture
{
public:
    ID id;

    // Special member functions
    Texture();
    Texture(const std::filesystem::path path, const unsigned min_filter, const unsigned mag_filter, const bool make_mipmap = false);
    ~Texture();

    // General
    void bind() const;
    void unbind() const;

private:
    Pixmap_t  *m_pixmap;
    PngHndl_t *m_png_hndl;
};
