#include <iostream>
#include <cassert>
#include <fstream>
#include <iterator>
#include <string>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/glew.h> // Put before other OpenGL library headers

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "../include/callbacks.hpp"

#define APP_TITLE "KingCraft"

// X11 variables
Display                *dpy;  // The target monitor/display 
Window                  root; // The parent window of our application's window 
Window                  win;  // The application's window
XVisualInfo            *vi;   // Struct containing additional info about the window
Colormap                cmap; // The color map of the window
XSetWindowAttributes    swa;  // Set attributes struct
XWindowAttributes       gwa;  // Get attributes struct 
XEvent                  xev;  // XEvent struct for handling X events 
GLXContext              glx;  // The OpenGL context for X11
GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

static unsigned CompileShader(unsigned type, const std::string source)
{
   unsigned int id = glCreateShader(type);
   const char *src = source.c_str(); // OpenGL requires a C-style string for glShaderSource() 
   glShaderSource(id, 1, &src, NULL); // src is a C-style string containing the shader source code
   glCompileShader(id); // Compile the shader 

   int result;
   glGetShaderiv(id, GL_COMPILE_STATUS, &result); // Get shader compilation status
   if (result == GL_FALSE) // Print out reason for failure
   {
      int length;
      glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
      char* message = (char*)alloca(length * sizeof(char));
      glGetShaderInfoLog(id, length, &length, message);
      std::cerr << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") 
         << " shader: " << message << std::endl;
      glDeleteShader(id);
      return 0;
   }

   return id;
}

static unsigned CreateShader(const std::string vertexShader, const std::string fragmentShader)
{
   unsigned int program = glCreateProgram(); // Create a shader program
   unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader); 
   unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

   glAttachShader(program, vs);
   glAttachShader(program, fs);
   glLinkProgram(program);
   glValidateProgram(program);

   glDeleteShader(vs);
   glDeleteShader(fs);

   return program;
}

int main(int argc, char *argv[]) 
{
   /*** Setup X11 window ***/

   dpy = XOpenDisplay(NULL); // NULL = first monitor

   if (dpy == NULL) 
   {
      std::cerr << "Cannot connect to X server" << std::endl;
      exit(-1);
   }

   root = DefaultRootWindow(dpy);

   vi = glXChooseVisual(dpy, 0, att);

   if (vi == NULL) 
   {
      std::cerr << "No appropriate visual found" << std::endl;
      exit(-1);
   } 
   else 
   {
      std::cerr << "Visual " << vi->visualid << " selected" << std::endl;
   }

   cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
   swa.colormap = cmap;
   // Handle the following events:
   swa.event_mask = (ExposureMask | KeyPressMask);

   // TODO: Query user's screen size for window size
   win = XCreateWindow(dpy, root, 0, 0, 600, 600, 0, vi->depth, 
         InputOutput, vi->visual, (CWColormap | CWEventMask), &swa);

   XMapWindow(dpy, win); 
   XStoreName(dpy, win, APP_TITLE);

   glx = glXCreateContext(dpy, vi, NULL, GL_TRUE); // Create the OpenGL context
   glXMakeCurrent(dpy, win, glx); // Attach the GLX context to the application window 

   // Must be placed after a valid OpenGL context has been made current 
   if (glewInit() != GLEW_OK) 
   {
      std::cerr << "Failed to initialize GLEW" << std::endl;
      exit(-1);
   }

#ifdef DEBUG
   std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
#endif
   
   /*** Setup debugging ***/

   // NOTE: You cannot OR these!!!
   glEnable(GL_DEPTH_TEST); 
   glEnable(GL_DEBUG_OUTPUT);
   glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
   if (glDebugMessageCallback)
      glDebugMessageCallback(debugCallback, nullptr); // Set callback function for debug messages
   else 
      std::cout << "WARNING: glDebugMessageCallback() is unavailable!" << std::endl;

   /*** Setup vertex/fragment shaders ***/

   std::ifstream ifs("res/shader/vertex.shader");
   // Iterate until EOF (hence the empty iterator)
   const std::string vertexShader((std::istreambuf_iterator<char>(ifs)), 
                                  (std::istreambuf_iterator<char>()));
   ifs.close();
   ifs.open("res/shader/fragment.shader");
   const std::string fragmentShader((std::istreambuf_iterator<char>(ifs)),
                                    (std::istreambuf_iterator<char>()));
   std::cout << fragmentShader << std::endl;
   ifs.close();

   unsigned int shader = CreateShader(vertexShader, fragmentShader);

   /*** Setup VAO, VBO, and EBO ***/

   float vertices[(3 * 4)] = {
       0.5f,  0.5f,  0.0f,
       0.5f, -0.5f,  0.0f,
      -0.5f, -0.5f,  0.0f,
      -0.5f,  0.5f,  0.0f
   };

   unsigned int indices[6] = {
      0, 1, 3,
      1, 2, 3
   };

   unsigned int vao, vbo, ebo;
   glGenVertexArrays(1, &vao);
   glGenBuffers(1, &vbo);
   glGenBuffers(1, &ebo);

   glBindVertexArray(vao);

   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
   glEnableVertexAttribArray(0);

   // Unbind array buffer + vertex array
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);

   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

   /*** Game loop ***/

   while (true) 
   {
      XNextEvent(dpy, &xev);

      switch (xev.type)
      {
         case Expose: // Window was being overlapped by another window, but is now exposed
         {
            XGetWindowAttributes(dpy, win, &gwa);
            glViewport(0, 0, gwa.width, gwa.height);
            glClearColor(0.2f, 0.4f, 0.4f, 1.0);
            glClear(GL_COLOR_BUFFER_BIT); 

            glUseProgram(shader); // Bind shader program for draw call
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 

            glXSwapBuffers(dpy, win); 
            break;
         }
         case KeyPress:
         {
            std::cout << "Key press detected" << std::endl;
            break;
         }
         case ButtonPress:
         {
            glXMakeCurrent(dpy, None, NULL);
            std::cout << "Click detected" << std::endl;
            break;
         }
      }
   } 

   glDeleteVertexArrays(1, &vao);
   glDeleteBuffers(1, &vbo);
   glDeleteBuffers(1, &ebo);
   glDeleteProgram(shader);

   return 0;
} 
