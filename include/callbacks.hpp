#pragma once

#include "common.hpp"

inline void GLAPIENTRY debugCallback(
   GLenum source,
   GLenum type,
   GLuint id,
   GLenum severity,
   GLsizei length,
   const GLchar *message,
   const void *userParam
) 
{
   std::cerr << "GL Callback: type = " << (type == GL_DEBUG_TYPE_ERROR ? "GL ERROR" : "")
      << " severity = " << severity << " message = " << message << std::endl;
}
