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

    glGenTextures(1, &this->id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->id);

    // Generate texture for each face of the cube map
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

    // Cube map parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Define skybox vertices
    const float scale = 200.0f;
    Vec4_t v0 = { .v = { -0.5f * scale, -0.5f * scale,  0.5f * scale, 0.0f }};
    Vec4_t v1 = { .v = { -0.5f * scale,  0.5f * scale,  0.5f * scale, 0.0f }};
    Vec4_t v2 = { .v = { -0.5f * scale, -0.5f * scale, -0.5f * scale, 0.0f }};
    Vec4_t v3 = { .v = { -0.5f * scale,  0.5f * scale, -0.5f * scale, 0.0f }};
    Vec4_t v4 = { .v = {  0.5f * scale, -0.5f * scale,  0.5f * scale, 0.0f }};
    Vec4_t v5 = { .v = {  0.5f * scale,  0.5f * scale,  0.5f * scale, 0.0f }};
    Vec4_t v6 = { .v = {  0.5f * scale, -0.5f * scale, -0.5f * scale, 0.0f }};
    Vec4_t v7 = { .v = {  0.5f * scale,  0.5f * scale, -0.5f * scale, 0.0f }};

    this->mesh.vertices = {
        // Right
        Vertex{ .pos = { v3.x, v3.y, v3.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v7.x, v7.y, v7.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v1.x, v1.y, v1.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v5.x, v5.y, v5.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v1.x, v1.y, v1.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v7.x, v7.y, v7.z }, .tex = {}, .rgb = {}},
        // Left
        Vertex{ .pos = { v6.x, v6.y, v6.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v2.x, v2.y, v2.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v4.x, v4.y, v4.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v0.x, v0.y, v0.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v4.x, v4.y, v4.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v2.x, v2.y, v2.z }, .tex = {}, .rgb = {}},
        // Back
        Vertex{ .pos = { v2.x, v2.y, v2.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v3.x, v3.y, v3.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v0.x, v0.y, v0.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v1.x, v1.y, v1.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v0.x, v0.y, v0.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v3.x, v3.y, v3.z }, .tex = {}, .rgb = {}},
        // Front
        Vertex{ .pos = { v7.x, v7.y, v7.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v6.x, v6.y, v6.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v5.x, v5.y, v5.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v4.x, v4.y, v4.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v5.x, v5.y, v5.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v6.x, v6.y, v6.z }, .tex = {}, .rgb = {}},
        // Bottom
        Vertex{ .pos = { v6.x, v6.y, v6.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v3.x, v3.y, v3.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v2.x, v2.y, v2.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v3.x, v3.y, v3.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v6.x, v6.y, v6.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v7.x, v7.y, v7.z }, .tex = {}, .rgb = {}},
        // Top
        Vertex{ .pos = { v0.x, v0.y, v0.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v1.x, v1.y, v1.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v4.x, v4.y, v4.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v5.x, v5.y, v5.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v4.x, v4.y, v4.z }, .tex = {}, .rgb = {}},
        Vertex{ .pos = { v1.x, v1.y, v1.z }, .tex = {}, .rgb = {}}
    };

    glGenVertexArrays(1, &this->mesh.vao);
    glBindVertexArray(this->mesh.vao);

    glGenBuffers(1, &this->mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->mesh.vbo);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Color attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Buffer data
    glBufferData(
        GL_ARRAY_BUFFER,
        this->mesh.vertices.size() * sizeof(Vertex),
        this->mesh.vertices.data(),
        GL_STATIC_DRAW
    );

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

SkyBox::~SkyBox()
{
    glDeleteTextures(1, &this->id);
    if (glIsBuffer(this->mesh.vbo))
    {
        glDeleteBuffers(1, &this->mesh.vbo);
    }
    if (glIsVertexArray(this->mesh.vao))
    {
        glDeleteVertexArrays(1, &this->mesh.vao);
    }
}
