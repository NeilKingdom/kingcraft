#include "skybox.hpp"

SkyBox::SkyBox() :
    id(0), mesh{}
{}

// TODO: Do cube textures support mipmaps?

SkyBox::SkyBox(
    const cube_map_textures_t textures,
    const unsigned min_filter,
    const unsigned mag_filter
)
{
    mat4 m_scale = {};
    PngHndl_t *png_hndl = nullptr;
    Pixmap_t  *pixmap = nullptr;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    for (auto i = textures.begin(); i != textures.end(); ++i)
    {
        png_hndl = imc_png_open(std::filesystem::absolute(*i).c_str());
        pixmap = imc_png_parse(png_hndl);

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + (i - textures.begin()),
            0, GL_RGBA, pixmap->width, pixmap->height,
            0, GL_RGBA, GL_UNSIGNED_BYTE, pixmap->data
        );

        // TODO: Destroy pixmap via library...
        std::free(pixmap->data);
        std::free(pixmap);
        imc_png_close(png_hndl);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Unbind texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    vec4 v0 = { -0.5f, -0.5f,  0.5f, 1.0f };
    vec4 v1 = { -0.5f,  0.5f,  0.5f, 1.0f };
    vec4 v2 = { -0.5f, -0.5f, -0.5f, 1.0f };
    vec4 v3 = { -0.5f,  0.5f, -0.5f, 1.0f };
    vec4 v4 = {  0.5f, -0.5f,  0.5f, 1.0f };
    vec4 v5 = {  0.5f,  0.5f,  0.5f, 1.0f };
    vec4 v6 = {  0.5f, -0.5f, -0.5f, 1.0f };
    vec4 v7 = {  0.5f,  0.5f, -0.5f, 1.0f };

    // Scale skybox
    lac_get_scalar_mat4(m_scale, 200.0f, 200.0f, 200.0f);

    lac_multiply_vec4_mat4(v0, v0, m_scale);
    lac_multiply_vec4_mat4(v1, v1, m_scale);
    lac_multiply_vec4_mat4(v2, v2, m_scale);
    lac_multiply_vec4_mat4(v3, v3, m_scale);
    lac_multiply_vec4_mat4(v4, v4, m_scale);
    lac_multiply_vec4_mat4(v5, v5, m_scale);
    lac_multiply_vec4_mat4(v6, v6, m_scale);
    lac_multiply_vec4_mat4(v7, v7, m_scale);

    // Static mesh coordinates
    std::array<float, 108> vertices = {
        // Right
        v3[0], v3[1], v3[2],
        v7[0], v7[1], v7[2],
        v1[0], v1[1], v1[2],
        v5[0], v5[1], v5[2],
        v1[0], v1[1], v1[2],
        v7[0], v7[1], v7[2],

        // Left
        v6[0], v6[1], v6[2],
        v2[0], v2[1], v2[2],
        v4[0], v4[1], v4[2],
        v0[0], v0[1], v0[2],
        v4[0], v4[1], v4[2],
        v2[0], v2[1], v2[2],

        // Back
        v2[0], v2[1], v2[2],
        v3[0], v3[1], v3[2],
        v0[0], v0[1], v0[2],
        v1[0], v1[1], v1[2],
        v0[0], v0[1], v0[2],
        v3[0], v3[1], v3[2],

        // Front
        v7[0], v7[1], v7[2],
        v6[0], v6[1], v6[2],
        v5[0], v5[1], v5[2],
        v4[0], v4[1], v4[2],
        v5[0], v5[1], v5[2],
        v6[0], v6[1], v6[2],

        // Bottom
        v6[0], v6[1], v6[2],
        v3[0], v3[1], v3[2],
        v2[0], v2[1], v2[2],
        v3[0], v3[1], v3[2],
        v6[0], v6[1], v6[2],
        v7[0], v7[1], v7[2],

        // Top
        v0[0], v0[1], v0[2],
        v1[0], v1[1], v1[2],
        v4[0], v4[1], v4[2],
        v5[0], v5[1], v5[2],
        v4[0], v4[1], v4[2],
        v1[0], v1[1], v1[2]
    };

    mesh.vertices = vertices.size();

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.vertices, vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Unbind VAO and VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

SkyBox::~SkyBox()
{
    // TODO: Destroy textures
}

void SkyBox::bind() const
{
    glBindVertexArray(mesh.vao);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

void SkyBox::unbind() const
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindVertexArray(0);
}
