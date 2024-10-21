#include "skybox.hpp"

#if 0
    // Dirt block
    Pixmap_t dirt_block_front  = atlas.get_pixmap_at_id(1);
    Pixmap_t dirt_block_back   = atlas.get_pixmap_at_id(1);
    Pixmap_t dirt_block_right  = atlas.get_pixmap_at_id(1);
    Pixmap_t dirt_block_left   = atlas.get_pixmap_at_id(1);
    Pixmap_t dirt_block_top    = atlas.get_pixmap_at_id(0);
    Pixmap_t dirt_block_bottom = atlas.get_pixmap_at_id(0);

    imc_pixmap_rotate_cw(&dirt_block_right);
    imc_pixmap_rotate_cw(&dirt_block_right);
    imc_pixmap_rotate_cw(&dirt_block_front);
    imc_pixmap_rotate_ccw(&dirt_block_back);

    Pixmap_t dirt_block[6] = {
        dirt_block_back, dirt_block_front, dirt_block_right,
        dirt_block_left, dirt_block_top, dirt_block_bottom
    };

    // Cube map texture
    unsigned dirt_block_texid;
    glGenTextures(1, &dirt_block_texid);
    glBindTexture(GL_TEXTURE_CUBE_MAP, dirt_block_texid);

    // OpenGL   -> liblac
    // (right)  -> (back)
    // (left)   -> (front)
    // (top)    -> (right)
    // (bottom) -> (left)
    // (front)  -> (top)
    // (back)   -> (bottom)
    for (int i = 0; i < KCConst::CUBE_FACES; ++i)
    {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB,
            dirt_block[i].pixmap.width,
            dirt_block[i].pixmap.height,
            0, GL_RGB, GL_UNSIGNED_BYTE,
            dirt_block[i].data
        );
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif

