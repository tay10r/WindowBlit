#include <window_blit/app.hpp>

namespace window_blit {

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

} // namespace window_blit
