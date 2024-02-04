#pragma once

#include "common.hpp"
#include "game_state.hpp"
#include "camera.hpp"

#include "../res/vendor/imgui/imgui.h"
#include "../res/vendor/imgui/imgui_impl_opengl3.h"
#include "../res/vendor/imgui/imgui_impl_x11.h"

// Forward function declarations

void initImGui(const xObjects &imObjs);
void processImGuiEvents(xObjects &imObjs);
void renderImGuiFrame(gameState &state, xObjects &imObjs, Camera &camera);
