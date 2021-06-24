#include "app.h"

App::App(GLFWwindow* window)
  : m_window(window)
{}

App::~App() = default;

GLFWwindow*
App::get_glfw_window() noexcept
{
  return m_window;
}

AppFactoryBase::~AppFactoryBase() = default;
