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
 * @param[in] win Reference to the application's window
 */
void init_imgui(const KCWindow &win)
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
void process_imgui_events(KCWindow &win)
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

void draw_minimap(KCWindow &win, Camera &camera, GameState &game)
{
    // Make collapsible
    if (ImGui::CollapsingHeader("Camera POV", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Create canvas for minimap
        ImVec2 canvas_size(512, 512);
        ImGui::BeginChild("Camera POV", canvas_size, true, ImGuiWindowFlags_AlwaysUseWindowPadding);

        CullingFrustum frustum = camera.get_frustum_coords(3);

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 win_pos = ImGui::GetWindowPos();
        ImVec2 win_size = ImGui::GetContentRegionAvail();

        ImVec2 v_eye = { win_pos.x + (win_size.x / 2.0f), win_pos.y + (win_size.y / 2.0f) };
        ImVec2 v_left = {
            frustum.v_left[1] - camera.v_eye[1] + v_eye.x,
            frustum.v_left[0] - camera.v_eye[0] + v_eye.y
        };
        ImVec2 v_right = {
            frustum.v_right[1] - camera.v_eye[1] + v_eye.x,
            frustum.v_right[0] - camera.v_eye[0] + v_eye.y
        };

        for (int i = 0; i < win_size.x; i += game.chunk_size)
        {
            float x_offset = (float)((int)(win_pos.x + camera.v_eye[1] + i) % (int)(win_pos.x + win_size.x));
            ImVec2 p1 = { x_offset, win_pos.y };
            ImVec2 p2 = { x_offset, win_pos.y + win_size.y };
            draw_list->AddLine(p1, p2, ImColor(255, 255, 255), 1);
        }

        draw_list->AddCircleFilled(v_eye, 3, ImColor(255, 0, 0));
        draw_list->AddCircleFilled(v_left, 3, ImColor(255, 0, 0));
        draw_list->AddCircleFilled(v_right, 3, ImColor(255, 0, 0));

        draw_list->AddLine(v_eye, v_left, ImColor(0, 255, 0), 3);
        draw_list->AddLine(v_eye, v_right, ImColor(0, 255, 0), 3);
        draw_list->AddLine(v_left, v_right, ImColor(0, 255, 0), 3);

        ImGui::EndChild();
    }
}

/**
 * @brief Renders the ImGui window to the screen.
 * @since 07-05-2024
 * @param[in] win Reference to the application's window
 * @param[in] camera A reference to a Camera object that manages the camera's state
 */
void render_imgui_frame(KCWindow &win, Camera &camera)
{
    GameState &game = GameState::get_instance();

    // Start a new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplX11_NewFrame();
    ImGui::NewFrame();

    // ImGui widgets
    ImGui::Begin("KingCraft");
    ImGui::SliderFloat("FOV", &game.fov, 89.0f, 91.0f);
    ImGui::SliderFloat("Camera X Pos", &camera.v_eye[0], camera.v_eye[0] - 1.0f, camera.v_eye[0] + 1.0f);
    ImGui::SliderFloat("Camera Y Pos", &camera.v_eye[1], camera.v_eye[1] - 1.0f, camera.v_eye[1] + 1.0f);
    ImGui::SliderFloat("Camera Z Pos", &camera.v_eye[2], camera.v_eye[2] - 1.0f, camera.v_eye[2] + 1.0f);
    ImGui::Checkbox("Game Running", &game.is_running);
    draw_minimap(win, camera, game);
    ImGui::End();

    // Render frame
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glXSwapBuffers(win.dpy, win.win);
}
