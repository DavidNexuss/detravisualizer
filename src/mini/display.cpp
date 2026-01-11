#include "display.hpp"
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <external/imgui/imgui.h>
#include <external/imgui/imgui_impl_glfw.h>
#include <external/imgui/imgui_impl_opengl3.h>

namespace display {

static Window* globalWindow = nullptr;
float          main_scale   = 1.0f;

struct GlfwWindow : public Window {
  GLFWwindow* handle   = nullptr;
  int         width    = 0;
  int         height   = 0;
  float       lastTime = 0;
  float       scrollY  = 0.0f;

  GlfwWindow(int w, int h, const char* title = "GLFW/glad Window") :
    width(w), height(h) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!handle) {
      throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(handle);

    glfwSwapInterval(1);

    glfwSetWindowUserPointer(handle, this);
    glfwSetScrollCallback(handle, scrollCallback);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplGlfw_InitForOpenGL(handle, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init("#version 330");

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      glfwDestroyWindow(handle);
      throw std::runtime_error("Failed to initialize glad");
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
  }


  static void scrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset) {
    auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
    if (self) {
      self->scrollY += static_cast<float>(yoffset);
    }
  }


  ~GlfwWindow() override {
    if (handle) {
      glfwDestroyWindow(handle);
    }
    std::cout << "GlfwWindow destroyed." << std::endl;
  }

  float getScroll() override {
    float value = scrollY;

    scrollY = 0.0f;

    return value;
  }

  bool clickOn() override {
    if (!handle) return false;
    return glfwGetMouseButton(handle, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
  }

  bool clickOnRight() override {
    if (!handle) return false;
    return glfwGetMouseButton(handle, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
  }

  float getDeltaTime() override {
    float currentTime = glfwGetTime();
    float delta       = currentTime - lastTime;
    lastTime          = currentTime;
    return delta;
  }

  int getX() override {
    double x, y;
    glfwGetCursorPos(handle, &x, &y);
    return x;
  }

  int getY() override {
    double x, y;
    glfwGetCursorPos(handle, &x, &y);
    return y;
  }

  int getWidth() override {
    glfwGetWindowSize(handle, &width, &height);
    return width;
  }

  int getHeight() override {
    glfwGetWindowSize(handle, &width, &height);
    return height;
  }

  void swapBuffers() override {
    if (handle) {
      glfwSwapBuffers(handle);
    }
  }

  bool shouldClose() override {
    return glfwWindowShouldClose(handle);
  }
};

void init() {
  std::cout << "Initializing GLFW..." << std::endl;
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }

  ImGui::CreateContext();
}

void dispose() {
  if (globalWindow) {
    delete globalWindow;
    globalWindow = nullptr;
  }

  std::cout << "Terminating GLFW..." << std::endl;
  glfwTerminate();
}

void poll() {
  glfwPollEvents();
}

Window* windowCreate() {
  if (globalWindow) {
    std::cerr << "Warning: A window already exists. Deleting the old one."
              << std::endl;
    delete globalWindow;
  }

  try {
    globalWindow = new GlfwWindow(800, 600);
    return globalWindow;
  } catch (const std::exception& e) {
    std::cerr << "Window creation failed: " << e.what() << std::endl;
    return nullptr;
  }
}
} // namespace display
