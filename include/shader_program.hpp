#pragma once

#include "common.hpp"

class ShaderProgram
{
public:
    unsigned id;

    // Special member functions
    ShaderProgram();
    ShaderProgram(const std::string vertex_src, const std::string fragment_src);
    ~ShaderProgram();

    // General
    void bind() const;
    void unbind() const;

private:
    unsigned compile(const unsigned type, const std::string source);
};
