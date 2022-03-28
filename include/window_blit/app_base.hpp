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

  virtual void render(GLuint texture_id, int w, int h) = 0;

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

  /// @brief Used to indicate to the shader how many samples have been measured
  /// by the path tracer.
  ///
  /// @param sample_weight Each color channel in the texture is multiplied by
  /// this value before being assigned to a fragment.
  ///                      The multiplication happens before any tone mapping or
  ///                      sRGB conversion.
  virtual void set_sample_weight(float sample_weight);

  /// @brief Sets whether or not tone mapping is applied.
  ///
  /// @param tone_mapping_mask The level at which to apply tone mapping to the fragment.
  virtual void set_tone_mapping(float tone_mapping_mask);

  /// @brief Sets whether or not a conversion from linear RGB to sRGB occurs.
  ///
  /// @param srgb_mask The level at which to use the sRGB conversion in the final image.
  virtual void set_srgb(float srgb_mask);

protected:
  void load_rgb(const float* rgb, int w, int h, GLuint texture_id);

  void load_rgb(const glm::vec3* rgb, int w, int h, GLuint texture_id);

  void load_rgb(const unsigned char* rgb, int w, int h, GLuint texture_id);

private:
  friend AppBaseImpl;

  AppBaseImpl* m_impl = nullptr;
};

} // namespace window_blit

#endif // WINDOW_BLIT_RT_APP_HPP_INCLUDED
