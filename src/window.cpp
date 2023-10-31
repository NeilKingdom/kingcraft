#include <iostream>
#include <cassert>
#include <fstream>
#include <iterator>
#include <cstring>
#include <string>
#include <chrono>
#include <algorithm>
#include <iomanip>

#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#include <GL/glew.h> // NOTE: Put before other OpenGL library headers
#include <GL/gl.h>
#include <GL/glx.h>  // X11-specific APIs

#include "../include/callbacks.hpp"
#include "../../liblac/include/matmath.h"

#define APP_TITLE "KingCraft"

using namespace std::chrono;

// X11 variables
Display                *dpy;  // The target monitor/display
Window                  win;  // The application's window
Screen                 *scrn; //
int                     scrn_id;
XVisualInfo            *vi;   // Struct containing additional info about the window
XWindowAttributes       gwa;  // Get window attributes struct
XEvent                  xev;  // XEvent stores the most-recently received event type
GLXContext              glx;  // The OpenGL context for X11

static float fov = lac_deg_to_rad(90.0f);
static float znear = 1.0f;
static float zfar = 1000.0f;

void CalculateFrameRate(int &fps, int &fps_inc, steady_clock::time_point &time_prev)
{
    auto time_now = steady_clock::now();
    auto time_elapsed = duration_cast<nanoseconds>(time_now - time_prev).count();
    constexpr long sec_as_nano = 1 * 1000 * 1000 * 1000;
    ++fps_inc; // Increment each frame

    if (time_elapsed > sec_as_nano) // Wait until at least a second has elapsed
    {
        time_prev = time_now;
        fps = fps_inc;
        fps_inc = 0;

        std::cout << "FPS: " << fps << std::endl;
    }
}

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int *);

static unsigned CompileShader(unsigned type, const std::string source)
{
   unsigned int id = glCreateShader(type);
   const char *src = source.c_str();
   glShaderSource(id, 1, &src, NULL);
   glCompileShader(id);

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

static bool isExtensionSupported(const char *extList, const char *extension) {
	const char *start, *where, *terminator;

	where = strchr(extension, ' ');
	if (where || *extension == '\0') {
		return false;
	}

   start=extList;
   while (true) {
		where = strstr(start, extension);
		if (!where) break;

		terminator = where + strlen(extension);
		if (where == start || *(where - 1) == ' ') {
			if (*terminator == ' ' || *terminator == '\0') {
				return true;
			}
		}

		start = terminator;
	}

	return false;
}

int main(int argc, char **argv)
{
   /*** Setup X11 window ***/

   // Open the display
   dpy = XOpenDisplay(NULL); // NULL = first monitor
   if (dpy == NULL) {
      std::cerr << "Cannot connect to X server" << std::endl;
      exit(-1);
   }
   scrn = DefaultScreenOfDisplay(dpy);
   scrn_id = DefaultScreen(dpy);

   // Check GLX version
   int vmajor, vminor = 0;
   glXQueryVersion(dpy, &vmajor, &vminor);

   // Specify what version of OpenGL we're using to the X11 extension (330 Core)
   int glx_attribs[] = {
      GLX_X_RENDERABLE,    True,
      GLX_DRAWABLE_TYPE,   GLX_WINDOW_BIT,
      GLX_RENDER_TYPE,     GLX_RGBA_BIT,
      GLX_X_VISUAL_TYPE,   GLX_TRUE_COLOR,
      GLX_RED_SIZE,        8,
      GLX_GREEN_SIZE,      8,
      GLX_BLUE_SIZE,       8,
      GLX_ALPHA_SIZE,      8,
      GLX_DEPTH_SIZE,      24,
      GLX_STENCIL_SIZE,    8,
      /*
       * NOTE: The buffer swap for double buffering is synchronized with your monitor's
       * vertical refresh rate (v-sync). Disabling double buffering effectively
       * unlocks the framerate as the buffer swaps no longer need to align with v-sync.
       */
      GLX_DOUBLEBUFFER,    True,
      None
   };

   // Create a framebuffer configuration
   int fbcount;
   GLXFBConfig *fbc = glXChooseFBConfig(dpy, win, glx_attribs, &fbcount);
   if (fbc == NULL)
   {
      std::cerr << "Failed to retrieve framebuffer" << std::endl;
      XCloseDisplay(dpy);
      exit(EXIT_FAILURE);
   }

   // Pick the FB config/visual with the most samples per-pixel
	std::cout << "Getting best XVisualInfo\n";
	int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
	for (int i = 0; i < fbcount; i++) {
		vi = glXGetVisualFromFBConfig(dpy, fbc[i]);
		if (vi != 0) {
			int samp_buf, samples;
			glXGetFBConfigAttrib(dpy, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
			glXGetFBConfigAttrib(dpy, fbc[i], GLX_SAMPLES, &samples);

			if (best_fbc < 0 || (samp_buf && samples > best_num_samp)) {
				best_fbc = i;
				best_num_samp = samples;
			}
			if (worst_fbc < 0 || !samp_buf || samples < worst_num_samp)
				worst_fbc = i;
			worst_num_samp = samples;
		}
		XFree(vi);
	}
	std::cout << "Best visual info index: " << best_fbc << std::endl;
	GLXFBConfig bestFbc = fbc[best_fbc];
	XFree(fbc);

   vi = glXGetVisualFromFBConfig(dpy, bestFbc);
   if (vi == NULL) {
      std::cerr << "No appropriate visual found" << std::endl;
      exit(-1);
   }
   if (scrn_id != vi->screen) {
      std::cout << "scrn_id(" << scrn_id << ") does not match vi->screen(" << vi->screen << ")" << std::endl;
      exit(-1);
   }

   /*** Open an X11 window ***/

	XSetWindowAttributes windowAttribs;
	windowAttribs.border_pixel = BlackPixel(dpy, scrn_id);
	windowAttribs.background_pixel = WhitePixel(dpy, scrn_id);
	windowAttribs.override_redirect = true;
	windowAttribs.colormap = XCreateColormap(dpy, RootWindow(dpy, scrn_id), vi->visual, AllocNone);
	windowAttribs.event_mask = (ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask);
	win = XCreateWindow(dpy, RootWindow(dpy, scrn_id), 0, 0, 600, 600, 0, vi->depth, InputOutput, vi->visual, (CWBackPixel | CWColormap | CWBorderPixel | CWEventMask), &windowAttribs);

	/*** Create GLX OpenGL context ***/

	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");

	const char *glxExts = glXQueryExtensionsString(dpy, scrn_id);
	std::cout << "Late extensions:\n\t" << glxExts << std::endl;
	if (glXCreateContextAttribsARB == 0) {
		std::cout << "glXCreateContextAttribsARB() not found" << std::endl;
	}

	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 3,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};

   if (!isExtensionSupported(glxExts, "GLX_ARB_create_context")) {
      glx = glXCreateNewContext(dpy, bestFbc, GLX_RGBA_TYPE, 0, true);
   } else {
      glx = glXCreateContextAttribsARB(dpy, bestFbc, 0, true, context_attribs);
   }
   XSync(dpy, false);

   // Verifying that context is a direct context
   if (!glXIsDirect(dpy, glx)) {
      std::cout << "Indirect GLX rendering context obtained" << std::endl;
   } else {
      std::cout << "Direct GLX rendering context obtained" << std::endl;
   }
   glXMakeCurrent(dpy, win, glx);

#ifdef DEBUG
   std::cout << "GL Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GL Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
#endif

   XClearWindow(dpy, win);
   XMapRaised(dpy, win);

   // NOTE: Must be placed after a valid OpenGL context has been made current
   if (glewInit() != GLEW_OK)
   {
      std::cerr << "Failed to initialize GLEW" << std::endl;
      exit(-1);
   }

   /*** Setup debugging ***/

   // NOTE: You cannot OR these!!!
   glEnable(GL_DEPTH_TEST); // Enable z-ordering via depth buffer
   glEnable(GL_CULL_FACE); // Cull faces which are not visible to the camera
   glDepthFunc(GL_LESS); // Culling algorithm (GL_LESS = lower zbuffer values are rendered on top)
   glCullFace(GL_FRONT); // Use if triangles are rasterized in clockwise ordering
   glEnable(GL_DEBUG_OUTPUT);
   glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
   if (glDebugMessageCallback)
      glDebugMessageCallback(debugCallback, nullptr); // Set callback function for debug messages
   else
      std::cerr << "WARNING: glDebugMessageCallback() is unavailable!" << std::endl;

   /*** Setup VAO, VBO, and EBO ***/

   float vertices[] = {
       // positions          // colors
       0.5f,  0.5f,  -0.5f,  0.3f, 0.7f, 0.6f,  // top right (front)
       0.5f, -0.5f,  -0.5f,  1.0f, 1.0f, 0.9f,  // bottom right (front)
      -0.5f, -0.5f,  -0.5f,  0.4f, 1.0f, 0.3f,  // bottom left (front)
      -0.5f,  0.5f,  -0.5f,  0.7f, 0.7f, 1.0f,  // top left (front)

       0.5f,  0.5f,   0.5f,  0.9f, 0.5f, 0.1f,  // top right (back)
       0.5f, -0.5f,   0.5f,  1.0f, 0.4f, 0.2f,  // bottom right (back)
      -0.5f, -0.5f,   0.5f,  0.2f, 1.0f, 0.3f,  // bottom left (back)
      -0.5f,  0.5f,   0.5f,  0.5f, 0.5f, 0.5f   // top left (back)
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

   unsigned int vao, vbo, ebo;
   glGenVertexArrays(1, &vao);
   glGenBuffers(1, &vbo);
   glGenBuffers(1, &ebo);

   glBindVertexArray(vao);

   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
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
   std::cout << fragmentShader << std::endl;
   ifs.close();

   unsigned int shader = CreateShader(vertexShader, fragmentShader);

   /*** Initialize some variables ***/

   float camera_trn_x = 0.0f;

   float theta_rad = 0.0f;
   float theta_deg = 0.0f;
   float trn_fact = -3.0f;

   mat4 trn_mat = { 0 };
   mat4 rot_mat = { 0 };
   mat4 model_mat = { 0 };
   mat4 view_mat = { 0 };
   mat4 proj_mat = { 0 };

   int fps_inc = 0;
   int fps = 0;

   time_point<steady_clock> time_prev = steady_clock::now();
   std::chrono::duration<long, std::ratio<1, 1000000>>::rep time_elapsed;

   /*** Game loop ***/

   while (true)
   {
      auto time_prev2 = steady_clock::now();

      /*** Process events ***/

      while (XPending(dpy) > 0)
      {
         XNextEvent(dpy, &xev); // Blocks until event is received

         // TODO: Programmatically set keyboard delay via X11 akin to xset (reduces choppiness)
         switch (xev.type)
         {
            case Expose: // Window was being overlapped by another window, but is now exposed
            {
               /* Set affine transform for viewport based on window width/height */
               XGetWindowAttributes(dpy, win, &gwa);
               glViewport(0, 0, gwa.width, gwa.height);
               break;
            }
            case (KeyPress | KeyRelease):
            {
               KeySym sym = XLookupKeysym(&xev.xkey, 0);
               switch (sym)
               {
                  case XK_a:
                  {
                     camera_trn_x += 0.006 * time_elapsed;
                     break;
                  }
                  case XK_d:
                  {
                     camera_trn_x -= 0.006 * time_elapsed;
                     break;
                  }
               }
               break;
            }
            case ButtonPress: // Mouse button pressed
            {
               std::cout << "Click detected" << std::endl;
               break;
            }
         }
      }

      /*** Render ***/

      glClearColor(0.2f, 0.4f, 0.4f, 1.0); // Set background color
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUseProgram(shader); // Bind shader program for draw call
      glBindVertexArray(vao);

      theta_deg += (1 % 360);
      theta_rad = lac_deg_to_rad(theta_deg);
      //t_fact -= 0.01;

      // Model matrix (translate to world space)
      lac_get_rotation_mat4(&rot_mat, theta_rad, (theta_rad * 0.9f), (theta_rad * 0.8f)); // Rotate
      lac_get_translation_mat4(&trn_mat, 0.0f, 0.0f, trn_fact); // Translate
      lac_dot_prod_mat4(trn_mat, rot_mat, &model_mat);

      int modelLocation = glGetUniformLocation(shader, "model");
      glUniformMatrix4fv(modelLocation, 1, GL_TRUE, model_mat);

      // TODO: Add inverse function to make camera behave properly
      // View matrix (translate to view space)
      lac_get_look_at_mat4(
         &view_mat,
         (const vec3){ 0.000001f, 0.000001f, 1.0f },
         (const vec3){ 0.0f, 0.0f, 0.0f },
         (const vec3){ 0.0f, 0.0f, 1.0f }
      );
      view_mat[3] = camera_trn_x;
      int viewLocation = glGetUniformLocation(shader, "view");
      glUniformMatrix4fv(viewLocation, 1, GL_TRUE, view_mat);

      // Projection matrix
      lac_get_projection_mat4(&proj_mat, ((float)gwa.height / (float)gwa.width), fov, znear, zfar);
      int projLocation = glGetUniformLocation(shader, "proj");
      glUniformMatrix4fv(projLocation, 1, GL_TRUE, proj_mat);

      glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);
      glXSwapBuffers(dpy, win);

      auto time_now = steady_clock::now();
      time_elapsed = duration_cast<milliseconds>(time_now - time_prev2).count();
      //CalculateFrameRate(fps, fps_inc, time_prev);
   }

   glDeleteVertexArrays(1, &vao);
   glDeleteBuffers(1, &vbo);
   glDeleteBuffers(1, &ebo);
   glDeleteProgram(shader);

   XDestroyWindow(dpy, win);
   XCloseDisplay(dpy);

   return EXIT_SUCCESS;
}
