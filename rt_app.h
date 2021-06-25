#pragma once

#include "app.h"

#include <glm/glm.hpp>

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

private:
  RtAppImpl* m_impl = nullptr;
};