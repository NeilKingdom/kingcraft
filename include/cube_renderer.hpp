#pragma once

#include "common.hpp"
#include "cube_renderer.hpp"

class CubeFactory
{
public:
    CubeFactory() = delete;
    ~CubeFactory();

    std::shared_ptr<CubeFactory> get_instance() const;

private:
    std::shared_ptr<CubeFactory> m_cube_factory;
};
