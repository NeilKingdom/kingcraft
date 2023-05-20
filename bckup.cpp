#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/glew.h> /* Put before other OpenGL library headers */

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#define APP_TITLE "KingCraft"
#define ASSERT(x) if (!(x)) assert(false)
#define GLCall(x) GLClearError();\
   x;\
   ASSERT(GLCheckError())

static void GLClearError() 
{
   while (glGetError() != GL_NO_ERROR) ;;
}

static bool GLCheckError()
{
   int error;
   while ((error = glGetError()) != GL_NO_ERROR)
   {
      printf("OpenGL Error ");
      switch(error) {
         case GL_INVALID_ENUM:
            printf("GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument.");
            break;
         case GL_INVALID_VALUE:
            printf("GL_INVALID_OPERATION: A numeric argument is out of range.");
            break;
         case GL_INVALID_OPERATION:
            printf("GL_INVALID_OPERATION: The specified operation is not allowed in the current state.");
            break;
         case GL_INVALID_FRAMEBUFFER_OPERATION:
            printf("GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.");
            break;
         case GL_OUT_OF_MEMORY:
            printf("GL_OUT_OF_MEMORY: There is not enough memory left to execute the command.");
            break;
         case GL_STACK_UNDERFLOW:
            printf("GL_STACK_UNDERFLOW: An attempt has been made to perform an operation that would cause an internal stack to underflow.");
            break;
         case GL_STACK_OVERFLOW :
            printf("GL_STACK_OVERFLOW: An attempt has been made to perform an operation that would cause an internal stack to overflow.");
            break;
         default :
            printf("Unrecognized error %d", error);
      }
      printf("\n");
      return false;
   }
   return true;
}

/* X11 variables */
Display                *dpy;  /* The target monitor/display */
Window                  root; /* The parent window of our application's window */
Window                  win;  /* The application's window */
XVisualInfo            *vi;   /* Struct containing additional info about the window */
Colormap                cmap; /* The color map of the window */
XSetWindowAttributes    swa;  /* Set attributes struct */
XWindowAttributes       gwa;  /* Get attributes struct */
XEvent                  xev;  /* XEvent struct for handling X events */
GLXContext              glx;  /* The OpenGL context for X11 */
GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

static unsigned int CompileShader(unsigned int type, const char *source)
{
   GLCall(unsigned int id = glCreateShader(type));
   const char *src = strdup(source);
   GLCall(glShaderSource(id, 1, &src, NULL));
   GLCall(glCompileShader(id));

   int result;
   GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
   if (result == GL_FALSE)
   {
      int length;
      GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
      char* message = (char*)alloca(length * sizeof(char));
      GLCall(glGetShaderInfoLog(id, length, &length, message));
      printf("Failed to compile %s shader: %s\n", (type == GL_VERTEX_SHADER ? "vertex" : "fragment"), message);
      GLCall(glDeleteShader(id));
      return 0;
   }

   return id;
}

static unsigned int CreateShader(const char *vertexShader, const char *fragmentShader)
{
   unsigned int program = glCreateProgram();
   unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
   unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

   GLCall(glAttachShader(program, vs));
   GLCall(glAttachShader(program, fs));
   GLCall(glLinkProgram(program));
   GLCall(glValidateProgram(program));

   GLCall(glDeleteShader(vs));
   GLCall(glDeleteShader(fs));

   return program;
}

int main(int argc, char *argv[]) 
{
   dpy = XOpenDisplay(NULL); /* NULL = first monitor */

   if (dpy == NULL) 
   {
      printf("\n\tcannot connect to X server\n\n");
      exit(0);
   }

   root = DefaultRootWindow(dpy);

   vi = glXChooseVisual(dpy, 0, att);

   if (vi == NULL) 
   {
      fprintf(stderr, "No appropriate visual found\n");
      exit(-1);
   } 
   else 
   {
      fprintf(stderr, "visual %p selected\n", (void *)vi->visualid); 
   }

   cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

   swa.colormap = cmap;
   /* Handle the following events: */
   swa.event_mask = (ExposureMask | KeyPressMask);

   /* TODO: Query user's screen size */
   win = XCreateWindow(dpy, root, 0, 0, 600, 600, 0, vi->depth, 
         InputOutput, vi->visual, (CWColormap | CWEventMask), &swa);

   XMapWindow(dpy, win);
   XStoreName(dpy, win, APP_TITLE);

   glx = glXCreateContext(dpy, vi, NULL, GL_TRUE); /* Create the OpenGL context */
   glXMakeCurrent(dpy, win, glx); /* Attach the GLX context to the application window */

   /* Must be placed after a valid OpenGL context has been made current */
   if (glewInit() != GLEW_OK) 
   {
      fprintf(stderr, "Failed to initalize GLEW\n");
      exit(-1);
   }

   glEnable(GL_DEPTH_TEST); /* Enable depth comparisons and update the depth buffer */

   float positions[6] = {
      -0.5f, -0.5f,
       0.5f, -0.5f,
       0.0f,  0.5f
   };

   unsigned int buffer;
   GLCall(glGenBuffers(1, &buffer));
   GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
   GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, positions, GL_STATIC_DRAW));

   GLCall(glEnableVertexAttribArray(0));
   GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));

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
   GLCall(glUseProgram(shader));

   GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

   while(true) 
   {
      XNextEvent(dpy, &xev);

      switch (xev.type)
      {
         case Expose:
         {
            XGetWindowAttributes(dpy, win, &gwa);
            glViewport(0, 0, gwa.width, gwa.height);
            GLCall(glClear(GL_COLOR_BUFFER_BIT)); /* Clear the color buffer */

            GLCall(glDrawArrays(GL_TRIANGLES, 0, 3)); 

            glXSwapBuffers(dpy, win); /* Draw new contents */
            break;
         }
         case KeyPress:
         {
            printf("Key press detected\n");
            break;
         }
         case ButtonPress:
         {
            glXMakeCurrent(dpy, None, NULL);
            printf("Click detected\n");
            break;
         }
      }
   } 

   return 0;
} 
