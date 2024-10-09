#pragma once

#include "common.hpp"
#include "game_state.hpp"
#include "camera.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_x11.h"

// Forward function declarations

void initImGui(const xObjects &imObjs);
void processImGuiEvents(xObjects &imObjs);
void renderImGuiFrame(GameState &state, xObjects &imObjs, Camera &camera);
