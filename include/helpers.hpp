#pragma once

#include "common.hpp"

namespace KC
{
    static bool v3_eq(const vec3 v_a, const vec3 v_b)
    {
        return v_a[0] == v_b[0] && v_a[1] == v_b[1] && v_a[2] == v_b[2];
    }

    static bool v3_eq(const std::array<float, 3> v_a, const vec3 v_b)
    {
        return v_a[0] == v_b[0] && v_a[1] == v_b[1] && v_a[2] == v_b[2];
    }

    static bool v3_eq(const vec3 v_a, const std::array<float, 3> v_b)
    {
        return v_a[0] == v_b[0] && v_a[1] == v_b[1] && v_a[2] == v_b[2];
    }

    static bool v3_eq(const std::array<float, 3> v_a, const std::array<float, 3> v_b)
    {
        return v_a[0] == v_b[0] && v_a[1] == v_b[1] && v_a[2] == v_b[2];
    }

    static void fps_callback()
    {
        // TODO: Use settings.is_running
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            int fps = frames_elapsed.exchange(0);
            std::cout << "FPS: " << fps << std::endl;
        }
    }
}
