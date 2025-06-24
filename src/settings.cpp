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

void Settings::draw_minimap(KCWindow &win, Camera &camera, const float zoom)
{
    bool show_map = false;

    // Make collapsible
    if (ImGui::CollapsingHeader("Camera POV", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("Show Heightmap", &show_map);

        // Create canvas for minimap
        ImVec2 canvas_size(512, 512);
        ImGui::BeginChild("Camera POV", canvas_size, true, ImGuiWindowFlags_AlwaysUseWindowPadding);

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 win_pos = ImGui::GetWindowPos();
        ImVec2 win_size = ImGui::GetContentRegionAvail();

        // Draw vertical grid lines
        int x_boundary = win_size.x + (chunk_size * zoom);
        for (int i = 0; i < x_boundary; i += (chunk_size * zoom))
        {
            int player_pos = win_pos.x - (camera.v_eye[1] * zoom) + i;
            float x_offset = (player_pos % x_boundary) + win_pos.x;

            ImVec2 p1 = { x_offset, win_pos.y };
            ImVec2 p2 = { x_offset, win_pos.y + win_size.y };
            draw_list->AddLine(p1, p2, ImColor(0, 255, 255), 1);
        }

        // Draw horizontal grid lines
        int y_boundary = win_size.y + (chunk_size * zoom);
        for (int i = 0; i < y_boundary; i += (chunk_size * zoom))
        {
            int player_pos = win_pos.y - (camera.v_eye[0] * zoom) + i;
            float y_offset = (player_pos % y_boundary) + win_pos.y;

            ImVec2 p1 = { win_pos.x, y_offset };
            ImVec2 p2 = { win_pos.x + win_size.x, y_offset };
            draw_list->AddLine(p1, p2, ImColor(0, 255, 255), 1);
        }

        // TODO
        if (show_map)
        {
        }

        // Render visible chunks

        Frustum2D frustum = camera.get_frustum2D(5);

        ImVec2 v_eye = { win_pos.x + (win_size.x / 2.0f), win_pos.y + (win_size.y / 2.0f) };
        ImVec2 v_left = {
            v_eye.x - (frustum.v_left[1] - camera.v_eye[1]) * zoom,
            v_eye.y - (frustum.v_left[0] - camera.v_eye[0]) * zoom
        };
        ImVec2 v_right = {
            v_eye.x - (frustum.v_right[1] - camera.v_eye[1]) * zoom,
            v_eye.y - (frustum.v_right[0] - camera.v_eye[0]) * zoom
        };

        // Top left-most point of the encapsulating grid square
        ssize_t min_x = std::min(std::min(v_eye[0], v_left[0]), v_right[0]);
        ssize_t min_y = std::min(std::min(v_eye[1], v_left[1]), v_right[1]);

        // Bottom right-most point of the encapsulating grid square
        ssize_t max_x = std::max(std::max(v_eye[0], v_left[0]), v_right[0]);
        ssize_t max_y = std::max(std::max(v_eye[1], v_left[1]), v_right[1]);

        min_x -= (min_x % chunk_size);
        min_y -= (min_y % chunk_size);
        max_x = (max_x + chunk_size) - ((max_x + chunk_size) % chunk_size);
        max_y = (max_y + chunk_size) - ((max_y + chunk_size) % chunk_size);

        // Rasterize chunks within camera's frustum
        for (ssize_t y = min_y; y < max_y; y += chunk_size)
        {
            for (ssize_t x = min_x; x < max_x; x += chunk_size)
            {
                // TODO: Redundant
                vec3 v_A = { v_eye[0], v_eye[1], 0.0f };
                vec3 v_B = { v_left[0], v_left[1], 0.0f };
                vec3 v_C = { v_right[0], v_right[1], 0.0f };

                vec3 vA_vC, vB_vA, vC_vB;
                vec3 p_vA, p_vB, p_vC;
                vec3 v_P = { (float)x, (float)y, 0.0f };
                vec3 v_cross = {};

                lac_subtract_vec3(vA_vC, v_A, v_C);
                lac_subtract_vec3(vB_vA, v_B, v_A);
                lac_subtract_vec3(vC_vB, v_C, v_B);

                lac_subtract_vec3(p_vA, v_P, v_A);
                lac_subtract_vec3(p_vB, v_P, v_B);
                lac_subtract_vec3(p_vC, v_P, v_C);

                lac_calc_cross_prod(v_cross, vA_vC, p_vC);
                if (v_cross[2] >= 0.0f)
                {
                    continue;
                }

                lac_calc_cross_prod(v_cross, vB_vA, p_vA);
                if (v_cross[2] >= 0.0f)
                {
                    continue;
                }

                lac_calc_cross_prod(v_cross, vC_vB, p_vB);
                if (v_cross[2] >= 0.0f)
                {
                    continue;
                }

                ImVec2 top_left = { v_P[0], v_P[1] };
                ImVec2 btm_right = { v_P[0] + (chunk_size * zoom), v_P[1] + (chunk_size * zoom) };
                draw_list->AddRectFilled(top_left, btm_right, ImColor(255, 0, 0));
            }
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
    draw_minimap(win, camera, 1.5f);
    ImGui::End();

    // Render frame
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glXSwapBuffers(win.dpy, win.win);
}
