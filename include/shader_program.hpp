#pragma once

#include "common.hpp"

class ShaderProgram
{
public:
    // Special member functions
    ShaderProgram() = delete;
    ShaderProgram(const std::string vertex_src, const std::string fragment_src);
    ~ShaderProgram();

    // General
    void bind() const;
    void unbind() const;

private:
    unsigned m_id;

    unsigned compile(const unsigned type, const std::string source);
};
