#include <glad/glad.h>
#include <GL/gl.h>
#include <vector>
#include <iostream>
#include "io.hpp"
#include <external/imgui/imgui.h>
#include <external/imgui/imgui_impl_glfw.h>
#include <external/imgui/imgui_impl_opengl3.h>

namespace graphics {

void init() {
}

void dispose() {
}

GLuint loadProgram(const char* fsPath, const char* vsPath) {
  GLuint program = glCreateProgram();
  GLuint fs      = glCreateShader(GL_FRAGMENT_SHADER);
  GLuint vs      = glCreateShader(GL_VERTEX_SHADER);

  std::vector<char> fsSource = io::fileread(fsPath);
  std::vector<char> vsSource = io::fileread(vsPath);

  fsSource.push_back('\0');
  vsSource.push_back('\0');

  const char* fsSrc = fsSource.data();
  const char* vsSrc = vsSource.data();

  glShaderSource(fs, 1, &fsSrc, nullptr);
  glShaderSource(vs, 1, &vsSrc, nullptr);

  glCompileShader(fs);
  GLint fsStatus;
  glGetShaderiv(fs, GL_COMPILE_STATUS, &fsStatus);
  if (fsStatus != GL_TRUE) {
    char log[1024];
    glGetShaderInfoLog(fs, sizeof(log), nullptr, log);

    std::cerr << "Fragment shader compilation failed:\n"
              << log << "\n";
    glDeleteShader(fs);
    glDeleteShader(vs);
    glDeleteProgram(program);
    throw std::runtime_error("Failed to compile fragment shader");
  }

  glCompileShader(vs);
  GLint vsStatus;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &vsStatus);
  if (vsStatus != GL_TRUE) {
    char log[1024];
    glGetShaderInfoLog(vs, sizeof(log), nullptr, log);

    std::cerr << "Vertex shader compilation failed:\n"
              << log << "\n";

    glDeleteShader(fs);
    glDeleteShader(vs);
    glDeleteProgram(program);
    throw std::runtime_error("Failed to compile vertex shader");
  }

  glAttachShader(program, fs);
  glAttachShader(program, vs);
  glLinkProgram(program);

  GLint linkStatus;
  glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
  if (linkStatus != GL_TRUE) {
    char log[1024];
    glGetProgramInfoLog(program, sizeof(log), nullptr, log);
    std::cerr << "Program linking failed:\n"
              << log << "\n";
    glDeleteShader(fs);
    glDeleteShader(vs);
    glDeleteProgram(program);
    throw std::runtime_error("Failed to link program");
  }

  glDetachShader(program, fs);
  glDetachShader(program, vs);
  glDeleteShader(fs);
  glDeleteShader(vs);

  return program;
}
} // namespace graphics
