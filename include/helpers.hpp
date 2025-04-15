#pragma once

#include "common.hpp"

static bool is_equal(const vec3 v_a, const vec3 v_b)
{
    return v_a[0] == v_b[0] && v_a[1] == v_b[1] && v_a[2] == v_b[2];
}

static bool is_equal(const std::array<float, 3> v_a, const vec3 v_b)
{
    return v_a[0] == v_b[0] && v_a[1] == v_b[1] && v_a[2] == v_b[2];
}

static bool is_equal(const vec3 v_a, const std::array<float, 3> v_b)
{
    return v_a[0] == v_b[0] && v_a[1] == v_b[1] && v_a[2] == v_b[2];
}

static bool is_equal(const std::array<float, 3> v_a, const std::array<float, 3> v_b)
{
    return v_a[0] == v_b[0] && v_a[1] == v_b[1] && v_a[2] == v_b[2];
}
