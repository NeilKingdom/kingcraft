#pragma once

#include "common.hpp"

#include "../res/vendor/imgui/include/imgui.h"
#include "../res/vendor/imgui/include/imgui_impl_opengl3.h"
#include "../res/vendor/imgui/include/imgui_impl_x11.h"

// Foward class declaration
class Camera;

class Settings
{
public:
    float fov    = 89.0f;
    float aspect = 16.0f / 9.0f;
    float znear  = 1.0f;
    float zfar   = 1000.0f;

    ssize_t chunk_size = 16;
    size_t render_distance = 8;

    unsigned long seed = 12345UL;
    bool is_running = true;

    // Special member functions
    ~Settings() = default;
    Settings(const Settings &game) = delete;
    Settings &operator=(const Settings &game) = delete;

    // General
    static Settings &get_instance();

    void init_imgui(const KCWindow &win);
    void process_imgui_events(KCWindow &win);
    void render_imgui_frame(KCWindow &win, Camera &camera);

private:
    // Special member functions
    Settings() = default;

    // General
    void draw_minimap(KCWindow &win, Camera &camera, const float zoom = 1.0f);
};
