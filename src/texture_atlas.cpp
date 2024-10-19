#include "texture_atlas.hpp"

TextureAtlas::TextureAtlas(const unsigned stride, const unsigned pitch) :
    m_stride(stride), m_pitch(pitch), m_pixmap(nullptr)
{}

TextureAtlas::TextureAtlas(const unsigned stride, const unsigned pitch, const std::string path) :
    m_stride(stride), m_pitch(pitch)
{
    load_atlas(path);
}

TextureAtlas::~TextureAtlas()
{
    //imc_png_close(m_png_hndl);
}

void TextureAtlas::load_atlas(const std::string path)
{
    m_png_hndl = imc_png_open(path.c_str());
    m_pixmap = imc_png_parse(m_png_hndl);
}

Pixmap_t TextureAtlas::get_pixmap_at_id(const uint8_t id) const
{
    unsigned cols, x_offset, y_offset;
    Pixmap_t pixmap = { m_stride * 3, m_pitch, 0, 3, 8, nullptr };
    Rgba_t rgba;

    pixmap.data = new uint8_t[m_stride * imc_sizeof_px(pixmap) * m_pitch];

    cols = m_pixmap->width / m_stride;
    x_offset = (id % cols) * m_stride;
    y_offset = (id / cols) * m_pitch;

    for (unsigned y = y_offset; y < (y_offset + m_pitch); ++y)
    {
        for (unsigned x = x_offset; x < (x_offset + m_stride); ++x)
        {
            rgba = imc_pixmap_psample(m_pixmap, x, y);
            ((Rgb_t*)pixmap.data)[((y - y_offset) * m_stride) + (x - x_offset)] = Rgb_t{ rgba.r, rgba.g, rgba.b };
        }
    }

    return pixmap;
}

