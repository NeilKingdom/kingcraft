#pragma once

#include "common.hpp"
#include "camera.hpp"
#include "constants.hpp"
#include "window.hpp"
#include "impl_imgui.hpp"
#include "game_state.hpp"
#include "shader_program.hpp"
#include "block_factory.hpp"
#include "chunk_factory.hpp"

#include "noise_generator.hpp"
#include "chunk_list.hpp"

static inline void GLAPIENTRY debug_callback(
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
