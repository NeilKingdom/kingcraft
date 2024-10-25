#pragma once

#include "common.hpp"

class ShaderProgram
{
public:
    ID id;

    // Special member functions
    ShaderProgram();
    ShaderProgram(const std::string vertex_src, const std::string fragment_src);
    ~ShaderProgram();

    // General
    void bind() const;
    void unbind() const;

private:
    // General
    unsigned compile(const unsigned type, const std::string src) const;
};
