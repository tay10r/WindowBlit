#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>

class App
{
public:
  static App* create_app(GLFWwindow* window);

  App(GLFWwindow* window);

  virtual ~App() = default;

  virtual void on_frame() = 0;

  virtual void on_key(int key, int scancode, int action, int mods) = 0;

  virtual void on_close() = 0;

protected:
  GLFWwindow* get_glfw_window() noexcept;

private:
  GLFWwindow* m_window;
};
