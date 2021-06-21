#include <glad/glad.h>

#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <memory>

#include <cstdlib>

#include "app.h"

namespace {

void
glfw_error_callback(int /* code */, const char* description)
{
  std::cerr << "GLFW error: " << description << std::endl;
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
#ifdef _WIN32
WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
#else
main()
#endif
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
    std::cerr << "Failed to create main window." << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);

  glfwSwapInterval(1);

  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  std::unique_ptr<App> app(App::create_app(window));

  glfwSetWindowUserPointer(window, app.get());

  glfwSetKeyCallback(window, glfw_key_callback);

  while (!glfwWindowShouldClose(window)) {

    app->on_frame();

    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  app->on_close();

  glfwDestroyWindow(window);

  glfwTerminate();

  return EXIT_SUCCESS;
}
