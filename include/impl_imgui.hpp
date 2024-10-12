#pragma once

#include "common.hpp"
#include "camera.hpp"
#include "game_state.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_x11.h"

// Forward function declarations

void init_imgui(const XObjects &im_objs);
void process_imgui_events(XObjects &im_objs);
void render_imgui_frame(XObjects &im_objs, Camera &camera);
