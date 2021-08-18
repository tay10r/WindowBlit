#pragma once

#ifndef WINDOW_BLIT_APP_HPP_INCLUDED
#define WINDOW_BLIT_APP_HPP_INCLUDED

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace window_blit {

class App
{
public:
  App(GLFWwindow* window);

  virtual ~App();

  virtual void on_frame() = 0;

  virtual void on_key(int key, int scancode, int action, int mods) = 0;

  virtual void on_cursor_button(int button, int action, int mods) = 0;

  virtual void on_cursor_motion(double x, double y) = 0;

  virtual void on_close() = 0;

  virtual void on_resize(int w, int h) = 0;

protected:
  GLFWwindow* get_glfw_window() noexcept;

private:
  GLFWwindow* m_window;
};

class AppFactoryBase
{
public:
  virtual ~AppFactoryBase();

  virtual App* create_app(GLFWwindow*) const = 0;
};

template<typename DerivedApp>
class AppFactory final : public AppFactoryBase
{
public:
  App* create_app(GLFWwindow* window) const override
  {
    return new DerivedApp(window);
  }
};

} // namespace window_blit

#endif // WINDOW_BLIT_APP_HPP_INCLUDED
