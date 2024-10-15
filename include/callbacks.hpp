#pragma once

#include "common.hpp"

inline void GLAPIENTRY debug_callback(
   GLenum source,
   GLenum type,
   GLuint id,
   GLenum severity,
   GLsizei length,
   const GLchar *msg,
   const void *args
)
{
   std::cerr
       << "GL Callback: "
       << "\ntype = " << (type == GL_DEBUG_TYPE_ERROR ? "GL ERROR" : "GL INFO")
       << "\nseverity = " << severity
       << "\nmessage = " << msg
       << std::endl;
}
