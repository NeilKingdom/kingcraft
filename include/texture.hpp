#pragma once

#include <png_parser.h>

#include "common.hpp"

typedef uint8_t rgb8_t;

class Atlas; // Forward declaration

class Texture
{
public:
    // TODO: Make private?
    std::vector<rgb8_t> m_pixmap;   // Pixmap data

    // Special member functions
    Texture() = delete;
    Texture(const Atlas atlas, const uint8_t id);
    ~Texture() = default;

    // General
    rgb8_t sample_texture(unsigned x, unsigned y) const;

private:
    uint8_t m_id;                    // ID into texture atlas
    unsigned m_width;                // Texture width
    unsigned m_height;               // Texture height
};

class Atlas
{
public:
    Pixmap_t  *m_pixmap;

    // Special member functions
    Atlas() = delete;
    Atlas(const unsigned stride, const unsigned pitch);
    Atlas(const unsigned stride, const unsigned pitch, const std::string path);
    ~Atlas();

    // General
    void load_atlas(const std::string path);
    std::vector<rgb8_t> get_pixmap_at_id(const uint8_t id) const;

private:
    unsigned   m_stride;
    unsigned   m_pitch;
    PngHndl_t *m_png_hndl;
};
