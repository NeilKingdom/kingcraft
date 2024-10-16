#include "texture.hpp"

Texture::Texture(const Atlas atlas, const uint8_t id) :
    m_id(id)
{
    m_pixmap = atlas.get_pixmap_at_id(id);
}

Atlas::Atlas(const unsigned stride, const unsigned pitch) :
    m_stride(stride), m_pitch(pitch), m_pixmap(nullptr)
{}

Atlas::Atlas(const unsigned stride, const unsigned pitch, const std::string path) :
    m_stride(stride), m_pitch(pitch)
{
    load_atlas(path);
}

Atlas::~Atlas()
{
    imc_png_close(m_png_hndl);
}

void Atlas::load_atlas(const std::string path)
{
    m_png_hndl = imc_png_open(path.c_str());
    m_pixmap = imc_png_parse(m_png_hndl);
}

std::vector<Rgb_t> Atlas::get_pixmap_at_id(const uint8_t id) const
{
    unsigned cols, x_offset, y_offset, stride;
    auto pixmap = std::vector<Rgb_t>();
    Rgba_t rgba;

    pixmap.reserve(m_stride * m_pitch);

    cols = m_pixmap->width / m_stride;
    x_offset = (id % cols) * m_stride;
    y_offset = (id / cols) * m_pitch;

    for (unsigned y = y_offset; y < (y_offset + m_pitch); ++y)
    {
        for (unsigned x = x_offset; x < (x_offset + m_stride); ++x)
        {
            rgba = imc_pixmap_psample(m_pixmap, x, y);
            pixmap.emplace_back(Rgb_t{ rgba.r, rgba.g, rgba.b });
        }
    }

    return pixmap;
}

