#include "main.hpp"

/**
 * @brief Cleanup all of the application's resources
 * @since 03-02-2024
 * @param glObjs An instance of glObjs containing OpenGL-related resources
 * @param xObjs An instance of xObjects containing X11-related resources
 * @param imObjs An optional instance of xObjects containing ImGui-related resources
 */
static void cleanup(
    GLObjects &gl_objs,
    XObjects &x_objs,
    const std::optional<XObjects> &im_objs
)
{
    // ImGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplX11_Shutdown();
    ImGui::DestroyContext();

    if (im_objs != std::nullopt)
    {
        XDestroyWindow(im_objs->dpy, im_objs->win);
        XFreeColormap(im_objs->dpy, im_objs->cmap);
        XCloseDisplay(im_objs->dpy);
    }

    // VAO, VBO, EBO
    glDeleteVertexArrays(1, &gl_objs.vao);
    glDeleteBuffers(1, &gl_objs.vbo);
    glDeleteBuffers(1, &gl_objs.ebo);
    glDeleteProgram(gl_objs.shader);

    // OpenGL context
    glXMakeCurrent(x_objs.dpy, None, NULL);
    glXDestroyContext(x_objs.dpy, x_objs.glx);

    // X11
    XDestroyWindow(x_objs.dpy, x_objs.win);
    XFreeColormap(x_objs.dpy, x_objs.cmap);
    XCloseDisplay(x_objs.dpy);
}

int main()
{
    using namespace std::chrono;

    /*** Variable declarations ***/

    Camera camera = Camera(90.0f, (9.0f / 16.0f), 1.0f, 1000.0f);
    Mvp mvp = Mvp(camera);

    XObjects x_objs;
    XObjects im_objs;
    GLObjects gl_objs;

    bool get_ptr_location = true;

    int curr_fps = 0;
    int frames_elapsed = 0;
    time_point<steady_clock> prev_fps = steady_clock::now();
    nanoseconds::rep time_elapsed = 0L;

    /*** Setup ***/

    GLXFBConfig best_fb_config = create_xwindow(x_objs, "KingCraft");
    create_opengl_context(x_objs, best_fb_config);
#ifdef DEBUG
    (void)create_xwindow(im_objs, "ImGui", 400, 400);
#endif

    // NOTE: Must be placed after a valid OpenGL context has been made current
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }

    init_imgui(im_objs);

    glEnable(GL_DEBUG_OUTPUT);      // Enable debug output
    glEnable(GL_CULL_FACE);         // Enable culling
    glEnable(GL_DEPTH_TEST);        // Enable z-ordering via depth buffer

    glCullFace(GL_FRONT);           // Culling algorithm (GL_FRONT = front faces, GL_BACK = back faces)
    glFrontFace(GL_CCW);            // Front faces (GL_CW = clockwise, GL_CCW = counter clockwise)
    glDepthFunc(GL_LESS);           // Depth algorithm (GL_LESS = lower zbuffer pixels are rendered on top)

    if (glDebugMessageCallback)
    {
        glDebugMessageCallback(debug_callback, nullptr);
    }
    else
    {
        std::cerr << "WARNING: glDebugMessageCallback() is unavailable!" << std::endl;
    }

    /*** Setup VAO, VBO, and EBO ***/


    /*
     *             z (up)
     * (forward) x |
     *            \|
     *  (left) y---+
     */
    float vertices[] = {
    //   Positions            Colors
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  // top left (front)
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  // top right (front)
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  1.0f,  // bottom left (front)
         0.5f,  0.5f, -0.5f,  1.0f,  1.0f,  0.0f,  // bottom right (front)

        -0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  1.0f,  // top left (back)
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  1.0f,  // top right (back)
        -0.5f, -0.5f, -0.5f,  1.0f,  1.0f,  1.0f,  // bottom left (back)
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f,  0.0f   // bottom right (back)
    };

    /*
    *   4____5
    *  /|   /|
    * 0-+--1 |
    * | 6__|_7
    * |/   |/
    * 2----3
    */
    unsigned int indices[] = {
        0, 3, 2,
        3, 0, 1,
        4, 1, 0,
        1, 4, 5,
        5, 6, 7,
        6, 5, 4,
        7, 6, 3,
        2, 3, 6,
        1, 7, 3,
        7, 1, 5,
        4, 2, 6,
        2, 4, 0
    };

    glGenVertexArrays(1, &gl_objs.vao);
    glGenBuffers(1, &gl_objs.vbo);
    glGenBuffers(1, &gl_objs.ebo);

    glBindVertexArray(gl_objs.vao);

    glBindBuffer(GL_ARRAY_BUFFER, gl_objs.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_objs.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind array buffer + vertex array
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Uncomment for wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    /*** Setup vertex/fragment shaders ***/

    auto ifs = std::ifstream();

    ifs.open("res/shader/vertex.shader");
    const std::string vertex_shader(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
    ifs.close();

    ifs.open("res/shader/fragment.shader");
    const std::string fragment_shader(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
    ifs.close();

    gl_objs.shader = create_shader(vertex_shader, fragment_shader);

    /*** Game loop ***/

    while (GameState::is_running)
    {
        auto frame_start_time = steady_clock::now();

        GameState::player.speed = Player::PLAYER_BASE_SPEED * (time_elapsed / (float)SEC_AS_NANO);

        process_events(x_objs, camera, get_ptr_location);
        render_frame(gl_objs, x_objs, camera, mvp, sizeof(indices));

        auto frame_end_time = steady_clock::now();
        time_elapsed = duration_cast<nanoseconds>(frame_end_time - frame_start_time).count();
        //CalculateFrameRate(fps, fpsCounter, timePrevFps);

#ifdef DEBUG
        // Switch OpenGL context to ImGui window
        glXMakeCurrent(im_objs.dpy, im_objs.win, x_objs.glx);
        process_imgui_events(im_objs);
        render_imgui_frame(im_objs, camera);
        glXMakeCurrent(x_objs.dpy, x_objs.win, x_objs.glx);
#endif
    }

#ifdef DEBUG
    cleanup(gl_objs, x_objs, im_objs);
#else
    cleanup(gl_objs, x_objs, std::nullopt);
#endif

    return EXIT_SUCCESS;
}
