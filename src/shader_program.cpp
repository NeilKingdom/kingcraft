/**
 * @file shader_program.cpp
 * @author Neil Kingdom
 * @version 1.0
 * @since 20-10-2024
 * @brief Creates an OpenGL shader program given GLSL code for fragment and vertex shaders.
 */

#include "shader_program.hpp"

/**
 * @brief Constructor for ShaderProgram which uses __vertex_path__ and __fragment_path__ for the program.
 * @since 20-10-2024
 * @param[in] vertex_path Path to the GLSL source code for the vertex shader
 * @param[in] fragment_path Path to the GLSL source code for the fragment shader
 */
ShaderProgram::ShaderProgram(
    const std::filesystem::path vertex_path,
    const std::filesystem::path fragment_path
)
{
    auto ifs = std::ifstream();
    std::string vertex_src;
    std::string fragment_src;

    ifs.open(std::filesystem::absolute(vertex_path));
    vertex_src = std::string(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
    ifs.close();

    ifs.open(std::filesystem::absolute(fragment_path));
    fragment_src = std::string(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
    ifs.close();

    id = glCreateProgram();
    ID vs = compile(GL_VERTEX_SHADER, vertex_src);
    ID fs = compile(GL_FRAGMENT_SHADER, fragment_src);

    glAttachShader(id, vs);
    glAttachShader(id, fs);
    glLinkProgram(id);
    glValidateProgram(id);

    glDeleteShader(vs);
    glDeleteShader(fs);
}

/**
 * @brief Default destructor for ShaderProgram
 * @since 20-10-2024
 */
ShaderProgram::~ShaderProgram()
{
    glUseProgram(0);
}

/**
 * @brief Binds the current shader program as the active one.
 * @since 20-10-2024
 */
void ShaderProgram::bind() const
{
    glUseProgram(id);
}

/**
 * @brief Unbinds the current shader program.
 * @since 20-10-2024
 */
void ShaderProgram::unbind() const
{
    glUseProgram(0);
}

/**
 * @brief Compiles a GLSL shader and returns its ID.
 * @since 02-03-2024
 * @param[in] type The type of shader being compiled (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
 * @param[in] source The GLSL source code for the shader
 * @returns The id of the compiled GLSL shader or 0 on failure
 */
ID ShaderProgram::compile(const unsigned type, const std::string src) const
{
    assert(type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER);

    int res;
    ID id = glCreateShader(type);
    const char *tmp_src = src.c_str();
    glShaderSource(id, 1, &tmp_src, NULL);
    glCompileShader(id);

    glGetShaderiv(id, GL_COMPILE_STATUS, &res);
    if (res == GL_FALSE)
    {
        int length;
        char *message = nullptr;

        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        glGetShaderInfoLog(id, length, &length, message);
        std::cerr << "Failed to compile "
            << ((type == GL_VERTEX_SHADER) ? "vertex" : "fragment")
            << " shader: " << std::string(message)
            << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}
