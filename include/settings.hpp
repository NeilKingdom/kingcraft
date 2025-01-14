#pragma once

#include "common.hpp"
#include "camera.hpp"
#include "game_state.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_x11.h"

// Forward function declarations

void init_imgui(const KCWindow &win);
void process_imgui_events(KCWindow &win);
void render_imgui_frame(KCWindow &win, Camera &camera);
