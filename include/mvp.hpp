#pragma once

#include "common.hpp"
#include "camera.hpp"

class Mvp
{
public:
    Mat4_t m_model;
    std::shared_ptr<Mat4_t> m_view;
    Mat4_t m_proj;

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

