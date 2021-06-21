#include "app.h"

App::App(GLFWwindow* window)
  : m_window(window)
{}

GLFWwindow*
App::get_glfw_window() noexcept
{
  return m_window;
}
