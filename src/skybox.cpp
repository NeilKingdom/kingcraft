#include "skybox.hpp"

SkyBox::SkyBox(
    const std::array<std::filesystem::path, KC::CUBE_FACES> tex_paths,
    const unsigned min_filter,
    const unsigned mag_filter,
    const bool make_mipmap
)
{
    PngHndl_t *png_hndl = nullptr;
    Pixmap_t *pixmap = nullptr;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    for (auto i = tex_paths.begin(); i != tex_paths.end(); ++i)
    {
        png_hndl = imc_png_open(std::filesystem::absolute(*i).c_str());
        pixmap = imc_png_parse(png_hndl);

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + (i - tex_paths.begin()),
            0, GL_RGBA, pixmap->width, pixmap->height,
            0, GL_RGBA, GL_UNSIGNED_BYTE, pixmap->data
        );

        if (make_mipmap)
        {
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        }

        imc_pixmap_destroy(pixmap);
        imc_png_close(png_hndl);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    Vec4_t v0 = { .v = { -0.5f, -0.5f,  0.5f, 1.0f }};
    Vec4_t v1 = { .v = { -0.5f,  0.5f,  0.5f, 1.0f }};
    Vec4_t v2 = { .v = { -0.5f, -0.5f, -0.5f, 1.0f }};
    Vec4_t v3 = { .v = { -0.5f,  0.5f, -0.5f, 1.0f }};
    Vec4_t v4 = { .v = {  0.5f, -0.5f,  0.5f, 1.0f }};
    Vec4_t v5 = { .v = {  0.5f,  0.5f,  0.5f, 1.0f }};
    Vec4_t v6 = { .v = {  0.5f, -0.5f, -0.5f, 1.0f }};
    Vec4_t v7 = { .v = {  0.5f,  0.5f, -0.5f, 1.0f }};

    // Scale skybox
    Mat4_t m_scale = qm_m4_scale(200.0f, 200.0f, 200.0f);

    v0 = qm_m4_v4_mul(m_scale, v0);
    v1 = qm_m4_v4_mul(m_scale, v1);
    v2 = qm_m4_v4_mul(m_scale, v2);
    v3 = qm_m4_v4_mul(m_scale, v3);
    v4 = qm_m4_v4_mul(m_scale, v4);
    v5 = qm_m4_v4_mul(m_scale, v5);
    v6 = qm_m4_v4_mul(m_scale, v6);
    v7 = qm_m4_v4_mul(m_scale, v7);

    // Static mesh coordinates
    std::array<VPos, 36> vertices = {
        // Right
        VPos{ v3.x, v3.y, v3.z },
        VPos{ v7.x, v7.y, v7.z },
        VPos{ v1.x, v1.y, v1.z },
        VPos{ v5.x, v5.y, v5.z },
        VPos{ v1.x, v1.y, v1.z },
        VPos{ v7.x, v7.y, v7.z },

        // Left
        VPos{ v6.x, v6.y, v6.z },
        VPos{ v2.x, v2.y, v2.z },
        VPos{ v4.x, v4.y, v4.z },
        VPos{ v0.x, v0.y, v0.z },
        VPos{ v4.x, v4.y, v4.z },
        VPos{ v2.x, v2.y, v2.z },

        // Back
        VPos{ v2.x, v2.y, v2.z },
        VPos{ v3.x, v3.y, v3.z },
        VPos{ v0.x, v0.y, v0.z },
        VPos{ v1.x, v1.y, v1.z },
        VPos{ v0.x, v0.y, v0.z },
        VPos{ v3.x, v3.y, v3.z },

        // Front
        VPos{ v7.x, v7.y, v7.z },
        VPos{ v6.x, v6.y, v6.z },
        VPos{ v5.x, v5.y, v5.z },
        VPos{ v4.x, v4.y, v4.z },
        VPos{ v5.x, v5.y, v5.z },
        VPos{ v6.x, v6.y, v6.z },

        // Bottom
        VPos{ v6.x, v6.y, v6.z },
        VPos{ v3.x, v3.y, v3.z },
        VPos{ v2.x, v2.y, v2.z },
        VPos{ v3.x, v3.y, v3.z },
        VPos{ v6.x, v6.y, v6.z },
        VPos{ v7.x, v7.y, v7.z },

        // Top
        VPos{ v0.x, v0.y, v0.z },
        VPos{ v1.x, v1.y, v1.z },
        VPos{ v4.x, v4.y, v4.z },
        VPos{ v5.x, v5.y, v5.z },
        VPos{ v4.x, v4.y, v4.z },
        VPos{ v1.x, v1.y, v1.z }
    };

    mesh.vertices.assign(vertices.begin(), vertices.end());

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(VPos), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

SkyBox::~SkyBox()
{
    glDeleteTextures(1, &id);
    if (glIsBuffer(mesh.vbo))
    {
        glDeleteBuffers(1, &mesh.vbo);
    }
    if (glIsVertexArray(mesh.vao))
    {
        glDeleteVertexArrays(1, &mesh.vao);
    }
}
