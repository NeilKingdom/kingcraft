/**
 * @file impl_imgui.cpp
 * @author Neil Kingdom
 * @version 1.0
 * @since 07-05-2024
 * @brief Provides functions for interacting with an ImGui debug window.
 */

#include "impl_imgui.hpp"

extern IMGUI_IMPL_API int ImGui_ImplX11_EventHandler(XEvent &event, XEvent *next_event);

/**
 * @brief Initializes the ImGui window with an X11 backend.
 * @since 07-05-2024
 * @param[in] im_objs Reference to and XObjects instance containing ImGui-related data
 */
void init_imgui(const XObjects &im_objs)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    //ImGuiIO &io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    // Initialize ImGui's backend for X11 and OpenGL
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplX11_Init(im_objs.dpy, (void*)im_objs.win);
}

/**
 * @brief Processes any pending X11 events for the ImGui window.
 * @since 07-05-2024
 * @param[in] im_objs Reference to and XObjects instance containing ImGui-related data
 */
void process_imgui_events(XObjects &im_objs)
{
    while (XPending(im_objs.dpy) > 0)
    {
        XNextEvent(im_objs.dpy, &im_objs.xev);
        ImGui_ImplX11_EventHandler(im_objs.xev, nullptr);

        // TODO: Expose events
        switch (im_objs.xev.type)
        {
            break;
        }
    }
}

/**
 * @brief Renders the ImGui window to the screen.
 * @since 07-05-2024
 * @param[in] im_objs Reference to and XObjects instance containing ImGui-related data
 * @param[in] camera A reference to a Camera object that manages the camera's state
 */
void render_imgui_frame(XObjects &im_objs, Camera &camera)
{
    bool is_running = GameState::is_running;
    float fov = GameState::fov;

    // Start a new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplX11_NewFrame();
    ImGui::NewFrame();

    // ImGui widgets
    ImGui::Begin("KingCraft");
    ImGui::SliderFloat("FOV", const_cast<float*>(&fov), 0.0f, 180.0f);
    ImGui::SliderFloat("Player Y Pos", &camera.v_eye[1], camera.v_eye[1] - 1.0f, camera.v_eye[1] + 1.0f);
    ImGui::Checkbox("Game Running", &is_running);
    ImGui::End();

    // Render frame
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glXSwapBuffers(im_objs.dpy, im_objs.win);

    GameState::is_running = is_running;
    GameState::fov = fov;
}
