/**
 * @file texture.cpp
 * @author Neil Kingdom
 * @version 1.0
 * @since 01-10-2024
 * @brief Fascilitates the creation of OpenGL textures.
 */

#include "texture.hpp"

/**
 * @brief Constructor for Texture which creates a texture from the PNG file specified by __path__.
 * @warning This class uses my own image loading library (libimc), which as of current date, only supports PNGs.
 * @since 01-10-2024
 * @param[in] path Path to the PNG file being used as a texture
 * @param[in] min_filter Scaling method used for downsampling
 * @param[in] mag_filter Scaling method used for upscaling
 * @param[in] make_mipmap Set to true if you want to generate a mipmap for the texture (default is false)
 */
Texture::Texture(
    const std::filesystem::path path,
    const unsigned min_filter,
    const unsigned mag_filter,
    const bool make_mipmap
)
{
    PngHndl_t *png_hndl = imc_png_open(std::filesystem::absolute(path).c_str());
    Pixmap_t *pixmap = imc_png_parse(png_hndl);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA,
        pixmap->width, pixmap->height,
        0, GL_RGBA, GL_UNSIGNED_BYTE,
        pixmap->data
    );

    imc_pixmap_destroy(pixmap);
    imc_png_close(png_hndl);

    if (make_mipmap)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

/**
 * @brief Default destuctor for Texture.
 * @since 01-10-2024
 */
Texture::~Texture()
{
    glDeleteTextures(1, &id);
}

/**
 * @brief Binds the current texture as the active one.
 * @since 01-10-2024
 */
void Texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, id);
}

/**
 * @brief Unbinds the current texture.
 * @since 01-10-2024
 */
void Texture::unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
