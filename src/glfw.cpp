#include <window_blit/glfw.hpp>

#include <window_blit/app.hpp>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#ifndef WINDOWBLIT_DISABLE_IMGUI
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

#include <iostream>
#include <memory>
#include <sstream>

#include <cstdlib>

namespace window_blit {

namespace {

void
glfw_error_callback(int /* code */, const char* description)
{
  std::cerr << "GLFW error: " << description << std::endl;
}

void
glfw_resize_callback(GLFWwindow* window, int w, int h)
{
  App* app = (App*)glfwGetWindowUserPointer(window);

  app->on_resize(w, h);
}

void
glfw_cursor_motion_callback(GLFWwindow* window, double x, double y)
{
  App* app = (App*)glfwGetWindowUserPointer(window);

  app->on_cursor_motion(x, y);
}

void
glfw_cursor_button_callback(GLFWwindow* window,
                            int button,
                            int action,
                            int mods)
{
  App* app = (App*)glfwGetWindowUserPointer(window);

  app->on_cursor_button(button, action, mods);
}

void
glfw_key_callback(GLFWwindow* window,
                  int key,
                  int scancode,
                  int action,
                  int mods)
{
  if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS))
    glfwSetWindowShouldClose(window, GLFW_TRUE);

  App* app = (App*)glfwGetWindowUserPointer(window);

  app->on_key(key, scancode, action, mods);
}

} // namespace

int
run_glfw_window(AppFactoryBase&& app_factory)
{
  if (glfwInit() != GLFW_TRUE) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return EXIT_FAILURE;
  }

  glfwSetErrorCallback(glfw_error_callback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  GLFWwindow* window = glfwCreateWindow(640, 480, "", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create main GLFW window" << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);

  glfwSwapInterval(1);

  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  int display_w = 0, display_h = 0;

  glfwGetFramebufferSize(window, &display_w, &display_h);

  glViewport(0, 0, display_w, display_h);

#ifndef WINDOWBLIT_DISABLE_IMGUI
  IMGUI_CHECKVERSION();

  ImGui::CreateContext();

  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window, true);

  ImGui_ImplOpenGL3_Init("#version 120");
#endif
  {
    // Scoped so that the smart pointer is destroyed before GLFW window.

    std::unique_ptr<App> app(app_factory.create_app(window));

    glfwSetWindowUserPointer(window, app.get());

    glfwSetKeyCallback(window, glfw_key_callback);

    glfwSetWindowSizeCallback(window, glfw_resize_callback);

    glfwSetCursorPosCallback(window, glfw_cursor_motion_callback);

    glfwSetMouseButtonCallback(window, glfw_cursor_button_callback);

    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window)) {

      glClearColor(0, 0, 0, 1);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glClear(GL_COLOR_BUFFER_BIT);

      glfwPollEvents();

#ifndef WINDOWBLIT_DISABLE_IMGUI
      ImGui_ImplOpenGL3_NewFrame();

      ImGui_ImplGlfw_NewFrame();

      ImGui::NewFrame();
#endif

      app->on_frame();

#ifndef WINDOWBLIT_DISABLE_IMGUI
      ImGui::Render();

      int display_w = 0, display_h = 0;

      glfwGetFramebufferSize(window, &display_w, &display_h);

      glViewport(0, 0, display_w, display_h);

      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

      glfwMakeContextCurrent(window);

      glfwSwapBuffers(window);
    }

    app->on_close();
  }

#ifndef WINDOWBLIT_DISABLE_IMGUI
  ImGui_ImplOpenGL3_Shutdown();

  ImGui_ImplGlfw_Shutdown();

  ImGui::DestroyContext();
#endif

  glfwDestroyWindow(window);

  glfwTerminate();

  return EXIT_SUCCESS;
}

} // namespace window_blit
