#pragma once
#include <glad/glad.h>
#include <GL/gl.h>
#include <external/imgui/imgui.h>
#include <external/imgui/imgui_internal.h>

namespace graphics {
void init();
void dispose();

GLuint loadProgram(const char* fsPath, const char* vsPath);
} // namespace graphics
