#pragma once

#include "common.hpp"
#include "camera.hpp"

class Mvp
{
public:
    mat4 m_model;
    std::shared_ptr<std::array<float, 16>> m_view;
    mat4 m_proj;

    /**
     * @brief Default constructor for MVP struct.
     * @since 23-03-2024
     * @param camera Camera to initialize the m_view field with.
     */
    Mvp(const Camera &camera) :
        m_model{},
        m_view(camera.m_view),
        m_proj{}
    {}
};

