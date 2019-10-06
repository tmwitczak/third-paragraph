#ifndef FIRST_PARAGRAPH_OPENGL_HEADERS_H
#define FIRST_PARAGRAPH_OPENGL_HEADERS_H
// //////////////////////////////////////////////////////////// Includes //
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)

#include <glad/glad.h>

#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

#include <GLFW/glfw3.h>
#include <iostream>

// ///////////////////////////////////////////////////////////////////// //
#endif //FIRST_PARAGRAPH_OPENGL_HEADERS_H
