#pragma once

#include "common.hpp"

inline void GLAPIENTRY debug_callback(
   GLenum source,
   GLenum type,
   GLuint id,
   GLenum severity,
   GLsizei length,
   const GLchar *message,
   const void *userParam
)
{
   std::cerr
       << "GL Callback: "
       << "\ntype = " << (type == GL_DEBUG_TYPE_ERROR ? "GL ERROR" : "N/A")
       << "\nseverity = " << severity
       << "\nmessage = " << message
       << std::endl;
}
