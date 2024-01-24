#include "../include/main.hpp"

void cleanup(xObjects &xObjs, glObjects &glObjs) 
{
    // ImGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplX11_Shutdown();
    ImGui::DestroyContext();

    // VAO, VBO, EBO 
    glDeleteVertexArrays(1, &glObjs.vao);
    glDeleteBuffers(1, &glObjs.vbo);
    glDeleteBuffers(1, &glObjs.ebo);
    glDeleteProgram(glObjs.shader);

    // OpenGL context
    glXMakeCurrent(xObjs.dpy, 0, 0);
    glXDestroyContext(xObjs.dpy, xObjs.glx);

    // X11
    XDestroyWindow(xObjs.dpy, xObjs.win);
    XFreeColormap(xObjs.dpy, xObjs.cmap);
    XCloseDisplay(xObjs.dpy);
}

int main() 
{
    using namespace std::chrono;

    /*** Variable declarations ***/

    Camera camera = Camera();
    Mvp mvp = Mvp(camera);

    xObjects xObjs;
    xObjects imObjs;
    glObjects glObjs;

    bool getPtrLocation = true;

    int fpsCounter = 0;
    int fps = 0;

    /*** Setup ***/

    (void)createXWindow(imObjs, "ImGui", 400, 400);
    GLXFBConfig bestFbConfig = createXWindow(xObjs, "KingCraft");
    createOpenGLContext(xObjs, bestFbConfig);

    // NOTE: Must be placed after a valid OpenGL context has been made current
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }

    glDepthFunc(GL_LESS);         // Culling algorithm (GL_LESS = lower zbuffer values are rendered on top)
    glEnable(GL_CULL_FACE);       // Enable culling
    glEnable(GL_DEPTH_TEST);      // Enable z-ordering via depth buffer
    glEnable(GL_DEBUG_OUTPUT);    // Enable debug output

    if (glDebugMessageCallback)
    {
        glDebugMessageCallback(debugCallback, nullptr); 
    }
    else
    {
        std::cerr << "WARNING: glDebugMessageCallback() is unavailable!" << std::endl;
    }

    /*** Setup VAO, VBO, and EBO ***/

    float vertices[] = {
         // positions         // colors
         0.5f,  0.5f, -0.5f,  0.3f,  0.7f,  0.6f,  // top right (front)
         0.5f, -0.5f, -0.5f,  1.0f,  1.0f,  0.9f,  // bottom right (front)
        -0.5f, -0.5f, -0.5f,  0.4f,  1.0f,  0.3f,  // bottom left (front)
        -0.5f,  0.5f, -0.5f,  0.7f,  0.7f,  1.0f,  // top left (front)

         0.5f,  0.5f,  0.5f,  0.9f,  0.5f,  0.1f,  // top right (back)
         0.5f, -0.5f,  0.5f,  1.0f,  0.4f,  0.2f,  // bottom right (back)
        -0.5f, -0.5f,  0.5f,  0.2f,  1.0f,  0.3f,  // bottom left (back)
        -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f   // top left (back)
    };

    /*
    *   7____4
    *  /|   /|
    * 3-+--0 |
    * | 6__|_5
    * |/   |/
    * 2----1
    */
    unsigned int indices[] = {
        3, 0, 2,
        0, 1, 2,
        7, 4, 3,
        4, 0, 3,
        6, 5, 7,
        5, 4, 7,
        2, 1, 6,
        1, 5, 6,
        0, 4, 1,
        4, 5, 1,
        7, 3, 6,
        3, 2, 6
    };

    glGenVertexArrays(1, &glObjs.vao);
    glGenBuffers(1, &glObjs.vbo);
    glGenBuffers(1, &glObjs.ebo);

    glBindVertexArray(glObjs.vao);

    glBindBuffer(GL_ARRAY_BUFFER, glObjs.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glObjs.ebo);
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

    std::ifstream ifs("res/shader/vertex.shader");
    const std::string vertexShader((std::istreambuf_iterator<char>(ifs)),
                                   (std::istreambuf_iterator<char>()));
    ifs.close();
    ifs.open("res/shader/fragment.shader");
    const std::string fragmentShader((std::istreambuf_iterator<char>(ifs)),
                                     (std::istreambuf_iterator<char>()));
    ifs.close();

    glObjs.shader = createShader(vertexShader, fragmentShader);

    /*** Game loop ***/

    time_point<steady_clock> timePrevFps = steady_clock::now();
    nanoseconds::rep elapsedTime = 0L;

    initImGui(imObjs.dpy, imObjs.win);

    while (true) 
    {
        auto frameStartTime = steady_clock::now();

        float playerSpeed = PLAYER_BASE_SPEED * (elapsedTime / (float)SEC_AS_NANO);
        camera.updateVelocity(playerSpeed);

        processEvents(xObjs, camera, getPtrLocation, playerSpeed);
        renderFrame(xObjs, glObjs, mvp, camera, sizeof(indices), imObjs.dpy, imObjs.win);

        auto frameEndTime = steady_clock::now();
        elapsedTime = duration_cast<nanoseconds>(frameEndTime - frameStartTime).count();
        //CalculateFrameRate(fps, fpsCounter, timePrevFps);
    }

    cleanup(xObjs, glObjs);
    return EXIT_SUCCESS;
}
