#pragma once

#include "common.hpp"

class Shader
{
public:
    ID id;

    // Special member functions
    Shader() = delete;
    Shader(const std::filesystem::path vertex_path, const std::filesystem::path fragment_path);
    ~Shader();

    // General
    void bind() const;
    void unbind() const;

private:
    // General
    ID compile(const unsigned type, const std::string src) const;
};
