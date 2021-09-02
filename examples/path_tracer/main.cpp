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
  glm::vec3 center = glm::vec3(0, 0, 0);

  float radius = 1;

  int material = 0;
};

struct SphereHit final
{
  float distance = std::numeric_limits<float>::infinity();

  operator bool() const noexcept
  {
    return distance != std::numeric_limits<float>::infinity();
  }
};

SphereHit
intersect(const Ray& ray, const Sphere& sphere)
{
  using namespace glm;
  using namespace std;

  auto oc = ray.org - sphere.center;

  auto a = dot(ray.dir, ray.dir);
  auto b = dot(oc, ray.dir);
  auto c = dot(oc, oc) - (sphere.radius * sphere.radius);
  auto disc = (b * b) - (a * c);

  if (disc < 0)
    return SphereHit{};

  auto discRoot = sqrt(disc);

  auto x0 = (-b - discRoot) / a;

  if ((x0 >= 0) && (x0 < ray.t_far))
    return SphereHit{ x0 };

  auto x1 = (-b + discRoot) / a;

  if ((x1 >= 0) && (x1 < ray.t_far))
    return SphereHit{ x1 };

  return SphereHit{};
}

Hit
to_hit(const Ray& ray,
       const Sphere& sphere,
       const SphereHit& sphere_hit,
       int primitive)
{
  const float bias = 0.001f;
  auto p = ray.org + (ray.dir * (sphere_hit.distance - bias));
  auto n = (p - sphere.center) / sphere.radius;
  return Hit{ primitive, sphere.material, p, n };
}

struct Material final
{
  glm::vec3 diffuse = glm::vec3(0.8, 0.8, 0.8);

  glm::vec3 emission = glm::vec3(0, 0, 0);
};

struct Pixel final
{
  glm::vec3 color;

  std::minstd_rand rng;

  Pixel(int seed = 3141)
    : color(0, 0, 0)
    , rng(seed)
  {}
};

class ExampleApp final : public window_blit::AppBase
{
public:
  ExampleApp(GLFWwindow* window);

  void render(float* rgb_buffer, int w, int h) override;

  void on_resize(int w, int h) override;

  void on_camera_change() override;

private:
  void reset();

  template<typename Rng>
  Ray generate_ray(glm::vec2 uv_min, glm::vec2 uv_max, float aspect, Rng& rng);

  Hit intersect_scene(const Ray& ray) const;

  template<typename Rng>
  glm::vec3 trace(const Ray& ray, Rng& rng, int depth = 0);

  glm::vec3 on_miss(const Ray& ray) const;

  template<typename Rng>
  glm::vec3 sample_unit_sphere(Rng& rng);

  void create_scene();

  std::vector<Pixel> m_accumulator;

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

  m_accumulator.resize(w * h);

  reset();

  create_scene();
}

void
ExampleApp::reset()
{
#pragma omp parallel for

  for (int i = 0; i < int(m_accumulator.size()); i++)
    m_accumulator[i] = Pixel(i);

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

  emissive_mat_a.emission = glm::vec3(1, 0, 1);
  emissive_mat_b.emission = glm::vec3(0, 1, 1);

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
ExampleApp::render(float* rgb_buffer, int w, int h)
{
  float aspect = float(w) / h;

  float rcp_w = 1.0f / w;
  float rcp_h = 1.0f / h;

  float sample_weight = 1.0f / (m_sample_count + 1.0f);

#pragma omp parallel for

  for (int i = 0; i < (w * h); i++) {

    int x = i % w;
    int y = i / w;

    glm::vec2 uv_min((x + 0.0f) * rcp_w, (y + 0.0f) * rcp_h);
    glm::vec2 uv_max((x + 1.0f) * rcp_w, (y + 1.0f) * rcp_h);

    auto ray = generate_ray(uv_min, uv_max, aspect, m_accumulator[i].rng);

    m_accumulator[i].color += trace(ray, m_accumulator[i].rng);

    rgb_buffer[(i * 3) + 0] = m_accumulator[i].color.x * sample_weight;
    rgb_buffer[(i * 3) + 1] = m_accumulator[i].color.y * sample_weight;
    rgb_buffer[(i * 3) + 2] = m_accumulator[i].color.z * sample_weight;
  }

  m_sample_count++;
}

void
ExampleApp::on_resize(int w, int h)
{
  m_accumulator.resize(w * h);

  reset();

  AppBase::on_resize(w, h);
}

template<typename Rng>
Ray
ExampleApp::generate_ray(glm::vec2 uv_min,
                         glm::vec2 uv_max,
                         float aspect,
                         Rng& rng)
{
  std::uniform_real_distribution<float> x_dist(uv_min.x, uv_max.x);
  std::uniform_real_distribution<float> y_dist(uv_min.y, uv_max.y);

  float fov_x = 0.5 * aspect;
  float fov_y = 0.5;

  float u = x_dist(rng);
  float v = y_dist(rng);

  glm::vec3 org = get_camera_position();

  glm::vec3 dir(
    ((2.0f * u) - 1.0f) * fov_x, (1.0f - (2.0f * v)) * fov_y, -1.0f);

  return Ray{ org, get_camera_rotation_transform() * glm::normalize(dir) };
}

template<typename Rng>
glm::vec3
ExampleApp::trace(const Ray& ray, Rng& rng, int depth)
{
  if (depth >= 3)
    return glm::vec3(0, 0, 0);

  auto hit = intersect_scene(ray);

  if (!hit)
    return on_miss(ray);

  auto refl_pos = hit.pos;

  auto refl_dir = glm::normalize(hit.nrm + sample_unit_sphere(rng));

  const auto& material = m_materials[hit.material];

  auto diffuse =
    trace(Ray{ refl_pos, refl_dir }, rng, depth + 1) * material.diffuse;

  return diffuse + material.emission;
}

template<typename Rng>
glm::vec3
ExampleApp::sample_unit_sphere(Rng& rng)
{
  std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

  for (;;) {

    glm::vec3 d(dist(rng), dist(rng), dist(rng));

    if (glm::dot(d, d) <= 1)
      return glm::normalize(d);
  }

  return glm::vec3(0, 1, 0);
}

glm::vec3
ExampleApp::on_miss(const Ray& ray) const
{
  const glm::vec3 up(0, 1, 0);

  float level = (glm::dot(ray.dir, up) + 1) * 0.5;

  glm::vec3 lo_color(1.0, 1.0, 1.0);

  glm::vec3 hi_color(0.5, 0.7, 1.0);

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
