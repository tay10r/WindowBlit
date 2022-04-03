#include <window_blit/window_blit.hpp>

#include "scene.hpp"

#include <glm/glm.hpp>

#include <imgui.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <algorithm>
#include <limits>
#include <random>
#include <vector>

#include <iostream>

namespace {

class ExampleApp final : public window_blit::AppBase
{
public:
  ExampleApp(GLFWwindow* window);

  void render(GLuint texture_id, int w, int h) override;

  void on_resize(int w, int h) override;

  void render_imgui() override
  {
    ImGui::InputInt("Sample Count", &m_sample_count, 1, 256);

    ImGui::InputInt("Resolution Divisor", &m_resolution_divisor, 1, 8);
  }

private:
  template<typename Rng>
  auto generate_ray(glm::vec2 uv_min, glm::vec2 uv_max, float aspect, Rng& rng) -> Ray;

  Scene m_scene;

  std::vector<glm::vec3> m_color;

  int m_resolution_divisor = 1;

  int m_sample_count = 4;
};

ExampleApp::ExampleApp(GLFWwindow* window)
  : AppBase(window)
{
  int w = 0;
  int h = 0;
  glfwGetWindowSize(window, &w, &h);

  on_resize(w, h);
}

void
ExampleApp::render(GLuint texture_id, int window_w, int window_h)
{
  const int w = window_w / m_resolution_divisor;
  const int h = window_h / m_resolution_divisor;

  const float aspect = float(w) / h;

  const float rcp_w = 1.0f / w;
  const float rcp_h = 1.0f / h;

#pragma omp parallel for

  for (int i = 0; i < (w * h); i++) {

    const int x = i % w;
    const int y = i / w;

    std::seed_seq seed{ x, y, w * h };

    std::minstd_rand rng(seed);

    const glm::vec2 uv_min((x + 0.0f) * rcp_w, (y + 0.0f) * rcp_h);
    const glm::vec2 uv_max((x + 1.0f) * rcp_w, (y + 1.0f) * rcp_h);

    glm::vec3 color{ 0, 0, 0 };

    for (int j = 0; j < m_sample_count; j++) {

      const auto ray = generate_ray(uv_min, uv_max, aspect, rng);

      color += m_scene.trace(ray, rng);
    }

    m_color[i] = color;
  }

  set_sample_weight(1.0f / m_sample_count);

  load_rgb(&m_color[0], w, h, texture_id);

  m_scene.advance();
}

void
ExampleApp::on_resize(int w, int h)
{
  w = w / m_resolution_divisor;
  h = h / m_resolution_divisor;

  m_color.resize(w * h);

  AppBase::on_resize(w, h);
}

template<typename Rng>
Ray
ExampleApp::generate_ray(glm::vec2 uv_min, glm::vec2 uv_max, float aspect, Rng& rng)
{
  std::uniform_real_distribution<float> x_dist(uv_min.x, uv_max.x);
  std::uniform_real_distribution<float> y_dist(uv_min.y, uv_max.y);

  const float fov_x = 0.5 * aspect;
  const float fov_y = 0.5;

  const float u = x_dist(rng);
  const float v = y_dist(rng);

  const glm::vec3 org = get_camera_position();

  const glm::vec3 dir(((2.0f * u) - 1.0f) * fov_x, (1.0f - (2.0f * v)) * fov_y, -1.0f);

  return Ray{ org, get_camera_rotation_transform() * glm::normalize(dir) };
}

} // namespace

int
#ifdef _WIN32
wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
#else
main()
#endif
{
  return window_blit::run_glfw_window(window_blit::AppFactory<ExampleApp>());
}
