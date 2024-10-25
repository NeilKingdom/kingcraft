#include "texture.hpp"

Texture::Texture() :
    id(0)
{}

Texture::Texture(
    const std::filesystem::path path,
    const unsigned min_filter,
    const unsigned mag_filter,
    const bool make_mipmap
)
{
    m_png_hndl = imc_png_open(path.c_str());
    m_pixmap = imc_png_parse(m_png_hndl);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB,
        m_pixmap->width, m_pixmap->height,
        0, GL_RGB, GL_UNSIGNED_BYTE,
        m_pixmap->data
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    if (make_mipmap)
    {
        glGenerateMipmap(id);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
    // TODO: destroy pixmap
    imc_png_close(m_png_hndl);
}

void Texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
