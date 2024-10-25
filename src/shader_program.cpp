#include "shader_program.hpp"

ShaderProgram::ShaderProgram() :
    id(0)
{}

ShaderProgram::ShaderProgram(const std::string vertex_src, const std::string fragment_src)
{
    ID program = glCreateProgram();
    ID vs = compile(GL_VERTEX_SHADER, vertex_src);
    ID fs = compile(GL_FRAGMENT_SHADER, fragment_src);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    id = program;
}

ShaderProgram::~ShaderProgram()
{
    glUseProgram(0);
}

void ShaderProgram::bind() const
{
    glUseProgram(id);
}

void ShaderProgram::unbind() const
{
    glUseProgram(0);
}

/**
 * @brief Compiles a GLSL shader and returns its id.
 * @since 02-03-2024
 * @param[in] type The type of shader being compiled (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
 * @param[in] source The GLSL source code for the shader
 * @returns The id of the compiled GLSL shader or 0 on failure
 */
unsigned ShaderProgram::compile(const unsigned type, const std::string src) const
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
