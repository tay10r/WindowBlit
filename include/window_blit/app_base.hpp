#pragma once

#ifndef WINDOW_BLIT_RT_APP_HPP_INCLUDED
#define WINDOW_BLIT_RT_APP_HPP_INCLUDED

#include <window_blit/app.hpp>

#include <glm/glm.hpp>

namespace window_blit {

class AppBaseImpl;

class AppBase : public App
{
public:
  AppBase(GLFWwindow* window);

  AppBase(const AppBase&) = delete;

  virtual ~AppBase();

  virtual void render(float* rgb_buffer, int w, int h) = 0;

  virtual void on_frame() override;

  virtual void on_key(int key, int scancode, int action, int mods) override;

  virtual void on_close() override;

  virtual void on_resize(int w, int h) override;

  virtual void on_camera_change();

  virtual glm::vec3 get_camera_position() const;

  virtual glm::mat3 get_camera_rotation_transform() const;

  virtual void on_cursor_button(int button, int action, int mods) override;

  void on_cursor_motion(double x, double y) override;

  virtual void on_cursor_motion(double x, double y, double dx, double dy);

  virtual void render_imgui();

private:
  friend AppBaseImpl;

  AppBaseImpl* m_impl = nullptr;
};

} // namespace window_blit

#endif // WINDOW_BLIT_RT_APP_HPP_INCLUDED
