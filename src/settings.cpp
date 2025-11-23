/**
 * @file impl_imgui.cpp
 * @author Neil Kingdom
 * @version 1.0
 * @since 07-05-2024
 * @brief Provides functions for interacting with an ImGui debug window.
 */

#include "settings.hpp"
#include "camera.hpp"

extern IMGUI_IMPL_API int ImGui_ImplX11_EventHandler(XEvent &event, XEvent *next_event);

Settings &Settings::get_instance()
{
    static Settings instance;
    return instance;
}

/**
 * @brief Initializes the ImGui window with an X11 backend.
 * @since 07-05-2024
 * @param[in] win Reference to the application's window
 */
void Settings::init_imgui(const KCWindow &win)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    //ImGuiIO &io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    // Initialize ImGui's backend for X11 and OpenGL
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplX11_Init(win.dpy, (void*)win.win);
}

/**
 * @brief Processes any pending X11 events for the ImGui window.
 * @since 07-05-2024
 * @param[in] win Reference to the application's window
 */
void Settings::process_imgui_events(KCWindow &win)
{
    while (XPending(win.dpy) > 0)
    {
        XNextEvent(win.dpy, &win.xev);
        ImGui_ImplX11_EventHandler(win.xev, nullptr);

        // TODO: Expose events
        switch (win.xev.type)
        {
            break;
        }
    }
}

/**
 * @brief Renders the ImGui window to the screen.
 * @since 07-05-2024
 * @param[in] win Reference to the application's window
 * @param[in] camera A reference to a Camera object that manages the camera's state
 */
void Settings::render_imgui_frame(KCWindow &win, Camera &camera)
{
    // Start a new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplX11_NewFrame();
    ImGui::NewFrame();

    // ImGui widgets
    ImGui::Begin("KingCraft");
    ImGui::SliderFloat("FOV", &fov, 89.0f, 91.0f);
    ImGui::SliderFloat("Camera X Pos", &camera.v_eye[0], camera.v_eye[0] - 1.0f, camera.v_eye[0] + 1.0f);
    ImGui::SliderFloat("Camera Y Pos", &camera.v_eye[1], camera.v_eye[1] - 1.0f, camera.v_eye[1] + 1.0f);
    ImGui::SliderFloat("Camera Z Pos", &camera.v_eye[2], camera.v_eye[2] - 1.0f, camera.v_eye[2] + 1.0f);
    ImGui::Checkbox("Game Running", &is_running);
    ImGui::End();

    // Render frame
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glXSwapBuffers(win.dpy, win.win);
}
