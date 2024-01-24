#pragma once 

#include "common.hpp"
#include "callbacks.hpp"
#include "camera.hpp"
#include "constants.hpp"
#include "window.hpp"
#include "impl_imgui.hpp"
#include "../res/vendor/imgui/imgui_impl_opengl3.h"
#include "../res/vendor/imgui/imgui_impl_x11.h"

/* Only transforms.h is required, but LSP gets confused */
#include <vecmath.h> 
#include <matmath.h>
#include <transforms.h>
