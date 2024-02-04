#include "../include/impl_imgui.hpp"

void initImGui(const xObjects &imObjs) 
{
    XEvent xev;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();

    // Initialize ImGui's backend for X11 and OpenGL
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplX11_Init(imObjs.dpy, (void*)imObjs.win); 
}

void renderImGuiFrame(const gameState &state, const xObjects &imObjs) 
{
    // Start a new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplX11_NewFrame();
    ImGui::NewFrame();

    // Render your ImGui content
    ImGui::Begin("KingCraft");
    ImGui::Text("This is some content.");
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glXSwapBuffers(imObjs.dpy, imObjs.win);
}
