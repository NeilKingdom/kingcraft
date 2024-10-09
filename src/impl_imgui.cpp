#include "impl_imgui.hpp"

extern IMGUI_IMPL_API int ImGui_ImplX11_EventHandler(XEvent &event, XEvent *next_event);

void initImGui(const xObjects &imObjs)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    // Initialize ImGui's backend for X11 and OpenGL
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplX11_Init(imObjs.dpy, (void*)imObjs.win);
}

void processImGuiEvents(xObjects &imObjs)
{
    while (XPending(imObjs.dpy) > 0)
    {
        XNextEvent(imObjs.dpy, &imObjs.xev);
        ImGui_ImplX11_EventHandler(imObjs.xev, nullptr);

        // TODO: Expose event
        switch (imObjs.xev.type)
        {
            break;
        }
    }
}

void updatePlayerPosition(Camera &camera)
{
    lac_add_vec3(&camera.vEye, camera.vEye, camera.vRightVel);
}

void renderImGuiFrame(GameState &state, xObjects &imObjs, Camera &camera)
{
    // Start a new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplX11_NewFrame();
    ImGui::NewFrame();

    // Render your ImGui content
    ImGui::Begin("KingCraft");
    ImGui::SliderFloat("FOV", &state.fov, 0.0f, 180.0f);
    ImGui::SliderFloat("Player X Pos", &camera.vEye[0], camera.vEye[0] - 1.0f, camera.vEye[0] + 1.0f);
    ImGui::Checkbox("Game Running", &state.isRunning);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glXSwapBuffers(imObjs.dpy, imObjs.win);
}
