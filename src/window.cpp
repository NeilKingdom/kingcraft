#include <iostream>
#include <cassert>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/glew.h> // Put before other OpenGL library headers

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "callbacks.hpp"

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
   glShaderSource(id, 1, &src, NULL);
   glCompileShader(id);

   int result;
   glGetShaderiv(id, GL_COMPILE_STATUS, &result);
   if (result == GL_FALSE)
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
   unsigned int program = glCreateProgram();
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

   // NOTE: You cannot OR these!!!
   glEnable(GL_DEPTH_TEST); 
   glEnable(GL_DEBUG_OUTPUT);
   glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
   if (glDebugMessageCallback)
      glDebugMessageCallback(debugCallback, nullptr);
   else 
      std::cout << "WARNING: glDebugMessageCallback() is unavailable!" << std::endl;

   float positions[6] = {
      -0.5f, -0.5f,
       0.5f, -0.5f,
       0.0f,  0.5f
   };

   unsigned int buffer;
   glGenBuffers(1, &buffer);
   glBindBuffer(GL_ARRAY_BUFFER, buffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, positions, GL_STATIC_DRAW);

   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

   const char *vertexShader = 
      "#version 330 core\n"
      "\n"
      "layout(location = 0) in vec4 position;\n"
      "void main()\n"
      "{\n"
      "   gl_Position = position;\n"
      "}\n";

   const char *fragmentShader =
      "#version 330 core\n"
      "\n"
      "layout(location = 0) out vec4 color;\n"
      "void main()\n"
      "{\n"
      "   color = vec4(1.0, 0.0, 0.0, 1.0);\n"
      "}\n";

   unsigned int shader = CreateShader(vertexShader, fragmentShader);
   glUseProgram(shader);

   glBindBuffer(GL_ARRAY_BUFFER, 0);

   while(true) 
   {
      XNextEvent(dpy, &xev);

      switch (xev.type)
      {
         case Expose:
         {
            XGetWindowAttributes(dpy, win, &gwa);
            glViewport(0, 0, gwa.width, gwa.height);
            glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer

            glDrawArrays(GL_TRIANGLES, 0, 3); 

            int error;
            while ((error = glGetError()) != GL_NO_ERROR)
               std::cout << "OpenGL Error: " << error << std::endl;

            glXSwapBuffers(dpy, win); // Draw new contents 
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

   return 0;
} 
