#pragma once

#include <glm/glm.hpp>

#include <random>
#include <vector>

struct Ray final
{
  glm::vec3 org = glm::vec3(0, 0, 0);

  glm::vec3 dir = glm::vec3(0, 0, -1);

  float t_far = std::numeric_limits<float>::infinity();
};

struct SphereHit final
{
  float distance = std::numeric_limits<float>::infinity();

  constexpr SphereHit() = default;

  constexpr SphereHit(float d)
    : distance(d)
  {}

  operator bool() const noexcept { return distance != std::numeric_limits<float>::infinity(); }
};

struct Sphere final
{
  glm::vec3 center{ 0, 0, 0 };

  float radius = 1;

  glm::vec3 color{ 0.8, 0.8, 0.8 };

  auto intersect(const Ray& ray) const -> SphereHit
  {
    using namespace glm;
    using namespace std;

    const auto oc = ray.org - center;

    const auto a = dot(ray.dir, ray.dir);
    const auto b = dot(oc, ray.dir);
    const auto c = dot(oc, oc) - (radius * radius);
    const auto disc = (b * b) - (a * c);

    if (disc < 0)
      return SphereHit{};

    const auto discRoot = sqrt(disc);

    const auto x0 = (-b - discRoot) / a;

    if ((x0 >= 0) && (x0 < ray.t_far))
      return SphereHit(x0);

    const auto x1 = (-b + discRoot) / a;

    if ((x1 >= 0) && (x1 < ray.t_far))
      return SphereHit(x1);

    return SphereHit{};
  }
};

struct Hit final
{
  float distance = std::numeric_limits<float>::infinity();

  glm::vec3 position{ 0, 0, 0 };

  glm::vec3 normal{ 0, 1, 0 };

  glm::vec3 color{ 0, 0, 0 };

  constexpr Hit(const SphereHit& sphere_hit, const Ray& ray, const Sphere& sphere)
    : distance(sphere_hit.distance)
    , position(ray.org + (ray.dir * sphere_hit.distance))
    , normal((position - sphere.center) / sphere.radius)
    , color(sphere.color)
  {}

  operator bool() const noexcept { return distance != std::numeric_limits<float>::infinity(); }

  bool operator<(const Hit& other) const noexcept { return distance < other.distance; }
};

struct Scene final
{
  std::vector<Sphere> objects;

  std::vector<Sphere> lights;

  glm::vec3 ambient_color{ 0.1, 0.1, 0.1 };

  float sun_distance = 5000;

  float shadow_bias = 1e-5f;

  float t = 0.0f;

  Scene()
  {
    const Sphere moon{ { 2000, 0, 0 }, 200, { 1, 1, 1 } };
    objects.emplace_back(moon);

    const Sphere ground{ { 0, -101, 0 }, 100 };
    objects.emplace_back(ground);

    const Sphere object{ { 0, 0, -2 }, 1 };
    objects.emplace_back(object);

    const Sphere sun{ glm::vec3(0, 0, -1) * sun_distance, 100, glm::vec3(0.98, 0.84, 0.11) * 100.0f };
    lights.emplace_back(sun);
  }

  template<typename Rng>
  auto trace(const Ray& ray, Rng& rng) const -> glm::vec3
  {
    const auto [object_hit, object_index] = intersect(objects, ray);
    if (!object_hit) {
      const auto [light_hit, object_index] = intersect(lights, ray);
      if (light_hit)
        return light_hit.color;
      else
        return sky(ray);
    }

    return shade_object(object_hit, rng);
  }

  void advance()
  {
    t += 1e-3f;

    const float theta = std::fmod(t, 1.0f) * M_PI * 2.0f;

    const float phi = 0;

    const float x = sun_distance * std::cos(phi) * std::sin(theta);
    const float y = sun_distance * std::sin(phi) * std::sin(theta);
    const float z = sun_distance * std::cos(theta);

    lights[0].center = glm::vec3(y, z, -x);
  }

private:
  template<typename Rng>
  auto shade_object(const Hit& hit, Rng& rng) const -> glm::vec3
  {
    const float shadow_bias = 1e-4f;

    const Ray occlusion_ray{ hit.position + (hit.normal * shadow_bias), hit.normal };

    const auto occlusion = occluded(objects, occlusion_ray);

    const glm::vec3 sky_color = occlusion ? ambient_color : sky(occlusion_ray);

    const glm::vec3 light_color = light_at(hit, rng);

    return hit.color * (sky_color + light_color);
  }

  template<typename Rng>
  auto light_at(const Hit& hit, Rng& rng) const -> glm::vec3
  {
    glm::vec3 result(0, 0, 0);

    for (const auto& light : lights) {

      const auto light_pos = sample_sphere(light, rng);

      const auto delta = light_pos - hit.position;

      const auto dir = glm::normalize(delta);

      const auto angle = glm::dot(dir, hit.normal);

      if (angle < 0.0f)
        continue;

      const auto ray = Ray{ hit.position + (dir * shadow_bias), dir };

      if (!occluded(objects, ray))
        result += (light.color * angle);
    }

    return result;
  }

  auto sky(const Ray& ray) const -> glm::vec3
  {
    const glm::vec3 up(0, 1, 0);

    const float level = (glm::dot(ray.dir, up) + 1) * 0.5;

    const glm::vec3 lo_color = glm::vec3(1.0, 1.0, 1.0);

    const glm::vec3 hi_color = glm::vec3(0.5, 0.7, 1.0);

    return (level * hi_color) + ((1 - level) * lo_color);
  }

  template<typename Rng>
  static auto sample_sphere(const Sphere& sphere, Rng& rng) -> glm::vec3
  {
    return sphere.center + (sample_unit_sphere(rng) * sphere.radius);
  }

  template<typename Rng>
  static auto sample_unit_sphere(Rng& rng) -> glm::vec3
  {
    std::uniform_real_distribution<float> dist(-1, 1);

    while (true) {
      const auto x = dist(rng);
      const auto y = dist(rng);
      const auto z = dist(rng);
      const glm::vec3 v(x, y, z);
      if (glm::dot(v, v) <= 1.0f)
        return glm::normalize(v);
    }

    return glm::vec3(0, 1, 0);
  }

  static auto occluded(const std::vector<Sphere>& spheres, const Ray& ray) -> bool
  {
    for (const auto& s : spheres) {
      if (s.intersect(ray))
        return true;
    }
    return false;
  }

  static auto intersect(const std::vector<Sphere>& spheres, const Ray& ray) -> std::pair<Hit, int>
  {
    int index = -1;

    SphereHit closest_sphere_hit;

    for (int i = 0; i < spheres.size(); i++) {

      const auto h = spheres[i].intersect(ray);

      if (h && (h.distance < closest_sphere_hit.distance)) {
        closest_sphere_hit = h;
        index = i;
      }
    }

    return { Hit(closest_sphere_hit, ray, spheres[index]), index };
  }
};
