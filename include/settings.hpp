#pragma once

#include "common.hpp"
#include "window.hpp"

#include "../res/vendor/imgui/include/imgui.h"
#include "../res/vendor/imgui/include/imgui_impl_opengl3.h"
#include "../res/vendor/imgui/include/imgui_impl_x11.h"

// Foward class declaration
class Camera;

class Settings
{
public:
    // Member variables
    float fov    = 89.0f;
    float aspect = 16.0f / 9.0f;
    float znear  = 0.1f;
    float zfar   = 1000.0f;
    size_t render_distance = 10;  // (in chunks)
    unsigned long seed = 12345UL;
    unsigned tgt_fps = 60;
    // TODO: Implement
    // bool cap_fps = true;
    bool is_running = true;

    // Special member functions
    Settings(const Settings &game) = delete;
    Settings &operator=(const Settings &game) = delete;
    Settings(Settings &&game) = delete;
    Settings &operator=(Settings &&game) = delete;

    // General
    static Settings &get_instance();
    void init_imgui(const KCWindow &win);
    void process_imgui_events(KCWindow &win);
    void render_imgui_frame(KCWindow &win, Camera &camera);

private:
    // Special member functions
    Settings() = default;
    ~Settings() = default;
};
