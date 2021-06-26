#pragma once

#ifndef BTN_RT_APP_H_INCLUDED
#define BTN_RT_APP_H_INCLUDED

#include <btn/app.h>

#include <glm/glm.hpp>

namespace btn {

class RtAppImpl;

class RtApp : public App
{
public:
  RtApp(GLFWwindow* window);

  RtApp(const RtApp&) = delete;

  virtual ~RtApp();

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

private:
  RtAppImpl* m_impl = nullptr;
};

} // namespace btn

#endif // BTN_RT_APP_H_INCLUDED
