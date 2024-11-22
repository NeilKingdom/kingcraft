#pragma once

#include "common.hpp"

class ShaderProgram
{
public:
    ID id;

    // Special member functions
    ShaderProgram() = delete;
    ShaderProgram(const std::filesystem::path vertex_path, const std::filesystem::path fragment_path);
    ~ShaderProgram();

    // General
    void bind() const;
    void unbind() const;

private:
    // General
    ID compile(const unsigned type, const std::string src) const;
};
