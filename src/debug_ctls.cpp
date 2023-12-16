// C++ APIs
#include <iostream>

// X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

// OpenGL
#include <GL/gl.h>   // General OpenGL APIs
#include <GL/glx.h>  // X11-specific OpenGL APIs

// ImGUI
#include "../res/vendor/imgui/imgui.h"
#include "../res/vendor/imgui/backends/imgui_impl_opengl3.h"
#include "../res/vendor/imgui/backends/imgui_impl_x11.h"

//void initImGUI(Window *win, Display *dpy) 
//{
//    XEvent xev;
//
//    // Initialize Dear ImGui
//    IMGUI_CHECKVERSION();
//    ImGui::CreateContext();
//    ImGuiIO& io = ImGui::GetIO();
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
//
//    // Initialize Dear ImGui's backend for X11 and OpenGL
//    ImGui_ImplX11_Init(dpy, win); 
//    ImGui_ImplOpenGL3_Init("#version 330"); 
//
//    while (true) {
//        while (XPending(dpy) > 0) 
//        {
//            XNextEvent(dpy, &xev);
//            switch (xev.type) 
//            {
//            }
//        }
//
//        // Start a new ImGui frame
//        ImGui_ImplOpenGL3_NewFrame();
//        ImGui_ImplX11_NewFrame();
//        ImGui::NewFrame();
//
//        // Render your ImGui content
//        ImGui::Begin("Hello, world!");
//        ImGui::Text("This is some content.");
//        ImGui::End();
//
//        // Rendering
//        glClearColor(1.0f, 1.0f, 1.0f, 0.9f);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//        ImGui::Render();
//        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
//
//        glXSwapBuffers(dpy, *win);
//    }
//}
