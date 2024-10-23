#include "texture.hpp"

Texture::Texture() :
    id(0)
{}

Texture::Texture(
    const Pixmap_t pixmap,
    const unsigned min_filter,
    const unsigned mag_filter,
    const bool make_mipmap
)
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pixmap.width, pixmap.height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixmap.data);
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

void Texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
