#pragma once

#include <png_parser.h>

#include "common.hpp"

class TextureAtlas
{
public:
    // Special member functions
    TextureAtlas() = delete;
    TextureAtlas(const unsigned stride, const unsigned pitch);
    TextureAtlas(const unsigned stride, const unsigned pitch, const std::string path);
    ~TextureAtlas();

    // General
    void load_atlas(const std::string path);
    Pixmap_t get_pixmap_at_id(const uint8_t id) const;

private:
    unsigned   m_stride;
    unsigned   m_pitch;
    Pixmap_t  *m_pixmap;
    PngHndl_t *m_png_hndl;
};
