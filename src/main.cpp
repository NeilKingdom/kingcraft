/**
 * @file main.cpp
 * @author Neil Kingdom
 * @version 1.0
 * @since 02-03-2024
 * @brief Entry point of the program.
 * Initializes everything and maintains the gameplay loop.
 */

#include "main.hpp"

/**
 * @brief Cleanup all of the application's resources.
 * @since 02-03-2024
 * @param[in] gl_objs An instance of GLObjects containing OpenGL-related resources
 * @param[in] x_objs An instance of XObjects containing X11-related resources
 * @param[in] im_objs An optional instance of XObjects containing ImGui-related resources
 */
static void cleanup(
    GLObjects &gl_objs,
    XObjects &x_objs,
    const std::optional<XObjects> &im_objs
)
{
    // ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplX11_Shutdown();

    if (im_objs != std::nullopt)
    {
        // OpenGL context
        ImGui::DestroyContext();

        // X11
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
    XFreeCursor(x_objs.dpy, x_objs.cur.cursor);
    XFreePixmap(x_objs.dpy, x_objs.cur.cpmap);

    XDestroyWindow(x_objs.dpy, x_objs.win);
    XFreeColormap(x_objs.dpy, x_objs.cmap);
    XCloseDisplay(x_objs.dpy);
}

int main()
{
    using namespace std::chrono;

    /*** Variable declarations ***/

    Atlas atlas(16, 16, "/home/neil/devel/projects/kingcraft/res/textures/texture_atlas.png");
    Texture texture = Texture(atlas, 1);

    Camera camera = Camera();
    Mvp mvp = Mvp(camera);

    XObjects x_objs;
    XObjects im_objs;
    GLObjects gl_objs;

    int frames_elapsed = 0;
    time_point<steady_clock> since = steady_clock::now();
    nanoseconds::rep frame_duration = 0L;

    /*** Window and OpenGL context initialization ***/

    GLXFBConfig best_fb_config = create_window(x_objs, "KingCraft", 1920, 1080);
    create_opengl_context(x_objs, best_fb_config);
#ifdef DEBUG
    (void)create_window(im_objs, "ImGui", 400, 400);
#endif

    init_imgui(im_objs);

    // Bind graphics drivers to OpenGL API specification
    // NOTE: Must be placed after a valid OpenGL context has been made current
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Setup callback function for when a OpenGL debug message is received
    if (glDebugMessageCallback)
    {
        glDebugMessageCallback(debug_callback, nullptr);
    }
    else
    {
        std::cerr << "WARNING: glDebugMessageCallback() is unavailable!" << std::endl;
    }

    glEnable(GL_DEBUG_OUTPUT);      // Enable debug output
    glEnable(GL_CULL_FACE);         // Enable culling
    glEnable(GL_DEPTH_TEST);        // Enable z-ordering via depth buffer

    glCullFace(GL_FRONT);           // Culling algorithm (GL_FRONT = front faces, GL_BACK = back faces)
    glFrontFace(GL_CCW);            // Front faces (GL_CW = clockwise, GL_CCW = counter clockwise)
    glDepthFunc(GL_LESS);           // Depth algorithm (GL_LESS = lower zbuffer pixels are rendered on top)

    /*** Setup VAO, VBO, and EBO ***/

    /*
     *             z (up)
     * (forward) x |
     *            \|
     *  (left) y---+
     */
    float vertices[] = {
    //   Positions            Texture coords
    //   X      Y      Z      U      V
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, // Top left (front)
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f, // Top right (front)
         0.5f, -0.5f, -0.5f,  0.0f,  1.0f, // Bottom left (front)
         0.5f,  0.5f, -0.5f,  1.0f,  1.0f, // Bottom right (front)

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, // Top left (back)
        -0.5f,  0.5f,  0.5f,  1.0f,  0.0f, // Top right (back)
        -0.5f, -0.5f, -0.5f,  0.0f,  1.0f, // Bottom left (back)
        -0.5f,  0.5f, -0.5f,  1.0f,  1.0f  // Bottom right (back)
    };

    /*
    *   4____5
    *  /|   /|
    * 0-+--1 |
    * | 6__|_7
    * |/   |/
    * 2----3
    *
    * NOTE: Must maintain a clockwise rotation so normals are calulated properly
    */
    unsigned indices[] = {
        0, 3, 2, 3, 0, 1, // Front face
        4, 1, 0, 1, 4, 5, // Top face
        5, 6, 7, 6, 5, 4, // Back face
        7, 6, 3, 2, 3, 6, // Bottom face
        1, 7, 3, 7, 1, 5, // Right face
        4, 2, 6, 2, 4, 0  // Left face
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind everything
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Uncomment for wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    /*** Generate textures ***/

    unsigned textures;
    glGenTextures(1, &textures);
    glBindTexture(GL_TEXTURE_2D, textures);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Dirt block (side)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, texture.m_pixmap.data());

    glBindTexture(GL_TEXTURE_2D, 0);

    /*** Create shader program(s) ***/

    auto ifs = std::ifstream();

    ifs.open("res/shader/vertex.shader");
    const std::string vertex_shader(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
    ifs.close();

    ifs.open("res/shader/fragment.shader");
    const std::string fragment_shader(std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()));
    ifs.close();

    gl_objs.shader = create_shader_prog(vertex_shader, fragment_shader);

    /*** Game loop ***/

    while (GameState::is_running)
    {
        auto frame_start = steady_clock::now();
        GameState::player.speed = Player::PLAYER_BASE_SPEED * (frame_duration / (float)SEC_AS_NANO);

        process_events(x_objs, camera);
        render_frame(gl_objs, x_objs, camera, mvp, sizeof(indices), textures);

        auto frame_end = steady_clock::now();
        frame_duration = duration_cast<nanoseconds>(frame_end - frame_start).count();
        //calculate_frame_rate(fps, frames_elapsed, since);

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
