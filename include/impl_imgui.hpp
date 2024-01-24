#pragma once

#include "common.hpp"
#include "../res/vendor/imgui/imgui.h"
#include "../res/vendor/imgui/imgui_impl_opengl3.h"
#include "../res/vendor/imgui/imgui_impl_x11.h"

// Forward function declarations

void initImGui(Display *dpy, Window win);
void renderImGuiFrame();
