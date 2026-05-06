#pragma once

#include "common.hpp"

class Shader
{
public:
    // Member variables
    ID id;

    // Special member functions
    Shader() = default;
    Shader(const std::filesystem::path vertex_path, const std::filesystem::path fragment_path);
    ~Shader();
    Shader(const Shader &shader) = default;
    Shader &operator=(const Shader &shader) = default;
    Shader(Shader &&shader) = default;
    Shader &operator=(Shader &&shader) = default;

    // General
    void bind() const;
    void unbind() const;

private:
    // General
    ID compile(const unsigned type, const std::string src) const;
};
