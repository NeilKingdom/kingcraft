// C++ APIs
#include <iostream>

// X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

// My APIs
#include "../include/debug_ctls.hpp"

// OpenGL
#include <GL/gl.h>   // General OpenGL APIs
#include <GL/glx.h>  // X11-specific OpenGL APIs

// ImGUI
#include "../res/vendor/imgui/imgui.h"
#include "../res/vendor/imgui/backends/imgui_impl_opengl3.h"
#include "../res/vendor/imgui/backends/imgui_impl_x11.h"

void initImGui(xObjects xObjs) 
{
    XEvent xev;

    // Initialize Dear ImGui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Initialize Dear ImGui's backend for X11 and OpenGL
    ImGui_ImplX11_Init(&xObjs.dpy, &xObjs.win); 
    ImGui_ImplOpenGL3_Init("#version 330");

    while (true) {
        while (XPending(xObjs.dpy) > 0) 
        {
            XNextEvent(xObjs.dpy, &xev);
            switch (xev.type) 
            {
            }
        }

        // Start a new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplX11_NewFrame();
        ImGui::NewFrame();

        // Render your ImGui content
        ImGui::Begin("Hello, world!");
        ImGui::Text("This is some content.");
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
