#include <window_blit/window_blit.hpp>

#include <glm/glm.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

#include <algorithm>
#include <limits>
#include <random>
#include <vector>

#include <iostream>

namespace {

struct Ray final
{
  glm::vec3 org = glm::vec3(0, 0, 0);

  glm::vec3 dir = glm::vec3(0, 0, -1);

  float t_far = std::numeric_limits<float>::infinity();
};

struct Hit final
{
  int primitive = -1;

  int material = 0;

  glm::vec3 pos;

  glm::vec3 nrm;

  operator bool() const noexcept { return primitive >= 0; }
};

struct Sphere final
{
  glm::vec3 center{ 0, 0, 0 };

  float radius = 1;

  int material = 0;
};

struct SphereHit final
{
  float distance = std::numeric_limits<float>::infinity();

  operator bool() const noexcept { return distance != std::numeric_limits<float>::infinity(); }
};

SphereHit
intersect(const Ray& ray, const Sphere& sphere)
{
  using namespace glm;
  using namespace std;

  const auto oc = ray.org - sphere.center;

  const auto a = dot(ray.dir, ray.dir);
  const auto b = dot(oc, ray.dir);
  const auto c = dot(oc, oc) - (sphere.radius * sphere.radius);
  const auto disc = (b * b) - (a * c);

  if (disc < 0)
    return SphereHit{};

  const auto discRoot = sqrt(disc);

  const auto x0 = (-b - discRoot) / a;

  if ((x0 >= 0) && (x0 < ray.t_far))
    return SphereHit{ x0 };

  const auto x1 = (-b + discRoot) / a;

  if ((x1 >= 0) && (x1 < ray.t_far))
    return SphereHit{ x1 };

  return SphereHit{};
}

Hit
to_hit(const Ray& ray, const Sphere& sphere, const SphereHit& sphere_hit, int primitive)
{
  const float bias = 0.001f;
  const auto p = ray.org + (ray.dir * (sphere_hit.distance - bias));
  const auto n = (p - sphere.center) / sphere.radius;
  return Hit{ primitive, sphere.material, p, n };
}

struct Material final
{
  glm::vec3 diffuse = glm::vec3(0.8, 0.8, 0.8);

  glm::vec3 emission = glm::vec3(0, 0, 0);
};

class ExampleApp final : public window_blit::AppBase
{
public:
  ExampleApp(GLFWwindow* window);

  void render(GLuint texture_id, int w, int h) override;

  void on_resize(int w, int h) override;

  void on_camera_change() override;

private:
  void reset();

  template<typename Rng>
  auto generate_ray(glm::vec2 uv_min, glm::vec2 uv_max, float aspect, Rng& rng) -> Ray;

  auto intersect_scene(const Ray& ray) const -> Hit;

  template<typename Rng>
  auto trace(const Ray& ray, Rng& rng, int depth = 0) -> glm::vec3;

  auto on_miss(const Ray& ray) const -> glm::vec3;

  template<typename Rng>
  auto sample_unit_sphere(Rng& rng) -> glm::vec3;

  void create_scene();

  std::vector<glm::vec3> m_accumulator;

  std::vector<std::minstd_rand> m_rngs;

  int m_resolution_divisor = 2;

  int m_sample_count = 0;

  std::vector<Sphere> m_spheres;

  std::vector<Material> m_materials;
};

ExampleApp::ExampleApp(GLFWwindow* window)
  : AppBase(window)
{
  int w = 0;
  int h = 0;
  glfwGetWindowSize(window, &w, &h);

  w /= m_resolution_divisor;
  h /= m_resolution_divisor;

  m_accumulator.resize(w * h);

  m_rngs.resize(w * h);

  reset();

  create_scene();
}

void
ExampleApp::reset()
{
  std::seed_seq seed{ int(m_rngs.size()), 1234 };

  std::mt19937 seed_rng(seed);

  for (int i = 0; i < int(m_rngs.size()); i++) {

    m_accumulator[i] = glm::vec3(0, 0, 0);

    std::seed_seq pixel_seed{ i, int(seed_rng()) };

    m_rngs[i] = std::minstd_rand(pixel_seed);
  }

  m_sample_count = 0;
}

void
ExampleApp::on_camera_change()
{
  reset();
}

void
ExampleApp::create_scene()
{
  Material diffuse_mat;
  Material emissive_mat_a;
  Material emissive_mat_b;

  emissive_mat_a.emission = glm::vec3(1, 0, 1) * 50.0f;
  emissive_mat_b.emission = glm::vec3(0, 1, 1) * 20.0f;

  m_materials.emplace_back(diffuse_mat);
  m_materials.emplace_back(emissive_mat_a);
  m_materials.emplace_back(emissive_mat_b);

  m_spheres.emplace_back(Sphere{ glm::vec3(-1.2, 0, -3), 0.5f });
  m_spheres.emplace_back(Sphere{ glm::vec3(0, 0, -3), 0.5f });
  m_spheres.emplace_back(Sphere{ glm::vec3(1.2, 0, -3), 0.5f });
  m_spheres.emplace_back(Sphere{ glm::vec3(0, -100.5, -3), 100 });

  // Assign the emissive materials to the first and third smaller spheres.

  m_spheres[0].material = 1;
  m_spheres[2].material = 2;
}

Hit
ExampleApp::intersect_scene(const Ray& ray) const
{
  int index = -1;

  SphereHit closest_sphere_hit;

  for (int i = 0; i < m_spheres.size(); i++) {

    auto h = intersect(ray, m_spheres[i]);

    if (h && (h.distance < closest_sphere_hit.distance)) {
      closest_sphere_hit = h;
      index = i;
    }
  }

  return to_hit(ray, m_spheres[index], closest_sphere_hit, index);
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

    const glm::vec2 uv_min((x + 0.0f) * rcp_w, (y + 0.0f) * rcp_h);
    const glm::vec2 uv_max((x + 1.0f) * rcp_w, (y + 1.0f) * rcp_h);

    const auto ray = generate_ray(uv_min, uv_max, aspect, m_rngs[i]);

    m_accumulator[i] += trace(ray, m_rngs[i]);
  }

  m_sample_count++;

  set_sample_weight(1.0f / m_sample_count);

  load_rgb(&m_accumulator[0], w, h, texture_id);
}

void
ExampleApp::on_resize(int w, int h)
{
  w = w / m_resolution_divisor;
  h = h / m_resolution_divisor;

  m_accumulator.resize(w * h);

  m_rngs.resize(w * h);

  reset();

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

template<typename Rng>
glm::vec3
ExampleApp::trace(const Ray& ray, Rng& rng, int depth)
{
  if (depth >= 3)
    return glm::vec3(0, 0, 0);

  const auto hit = intersect_scene(ray);

  if (!hit)
    return on_miss(ray);

  const auto refl_pos = hit.pos;

  const auto refl_dir = glm::normalize(hit.nrm + sample_unit_sphere(rng));

  const auto& material = m_materials[hit.material];

  const auto diffuse = trace(Ray{ refl_pos, refl_dir }, rng, depth + 1) * material.diffuse;

  return diffuse + material.emission;
}

template<typename Rng>
glm::vec3
ExampleApp::sample_unit_sphere(Rng& rng)
{
  std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

  for (;;) {

    const glm::vec3 d(dist(rng), dist(rng), dist(rng));

    if (glm::dot(d, d) <= 1)
      return glm::normalize(d);
  }

  return glm::vec3(0, 1, 0);
}

glm::vec3
ExampleApp::on_miss(const Ray& ray) const
{
  const glm::vec3 up(0, 1, 0);

  const float level = (glm::dot(ray.dir, up) + 1) * 0.5;

  const glm::vec3 lo_color = glm::vec3(1.0, 1.0, 1.0);

  const glm::vec3 hi_color = glm::vec3(0.5, 0.7, 1.0);

  return (level * hi_color) + ((1 - level) * lo_color);
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
