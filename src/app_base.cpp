#include <window_blit/app_base.hpp>

#include "shader.hpp"

#include "stb_image_write.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <imgui.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#define _USE_MATH_DEFINES 1

#include <cmath>
#include <cstdint>

#ifndef M_PI
#define M_PI 3.1415f
#endif

namespace window_blit {

namespace {

const char* g_vert_shader = R"(
#version 120

attribute vec2 g_pos;

varying vec2 g_tex_coord;

void
main()
{
  g_tex_coord = vec2((g_pos.x + 1) * 0.5, (1 - g_pos.y) * 0.5);

  gl_Position = vec4(g_pos, 0.0, 1.0);
}
)";

const char* g_frag_shader = R"(
#version 120

uniform sampler2D texture;

varying vec2 g_tex_coord;

void main()
{
  gl_FragColor = texture2D(texture, g_tex_coord);
}
)";

struct Frame final
{
  std::vector<float> color;
  int w = 0;
  int h = 0;

  void resize(int ww, int hh)
  {
    w = 0;
    h = 0;
    color.clear();

    color.resize(ww * hh * 3);

    w = ww;
    h = hh;
  }
};

class Camera
{
public:
  virtual ~Camera() = default;

  virtual void handle_key(int key, int action) = 0;

  virtual void handle_relative_motion(float dx, float dy) = 0;

  /// Indicates, based on the move state, if the camera will move on the next
  /// frame.
  virtual bool is_moving() const = 0;

  /// Performs translation based on move state.
  virtual void move() = 0;

  virtual glm::vec3 get_position() const = 0;

  virtual glm::mat3 get_rotation_transform() const = 0;
};

class FirstPersonCamera : public Camera
{
public:
  bool is_moving() const override
  {
    return m_up_speed || m_left_speed || m_down_speed || m_right_speed;
  }

  void move() override
  {
    glm::vec3 delta(0, 0, 0);

    delta += glm::vec3(0, 0, -1) * m_up_speed;
    delta += glm::vec3(-1, 0, 0) * m_left_speed;
    delta += glm::vec3(0, 0, 1) * m_down_speed;
    delta += glm::vec3(1, 0, 0) * m_right_speed;

    auto world_delta = get_rotation_transform() * glm::normalize(delta);

    m_position += world_delta * m_move_speed;
  }

  void handle_key(int key, int action) override
  {
    switch (key) {
      case GLFW_KEY_W:
        m_up_speed = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
        break;
      case GLFW_KEY_A:
        m_left_speed = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
        break;
      case GLFW_KEY_S:
        m_down_speed = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
        break;
      case GLFW_KEY_D:
        m_right_speed = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
        break;
    }
  }

  void handle_relative_motion(float dx, float dy) override
  {
    m_angle_x += dy * M_PI;
    m_angle_y += dx * M_PI;
  }

  glm::vec3 get_position() const override { return m_position; }

  glm::mat3 get_rotation_transform() const override
  {
    return glm::rotate(m_angle_y, glm::vec3(0, 1, 0)) *
           glm::rotate(m_angle_x, glm::vec3(1, 0, 0));
  }

private:
  float m_move_speed = 0.05;

  float m_up_speed = 0.0f;

  float m_left_speed = 0.0f;

  float m_down_speed = 0.0f;

  float m_right_speed = 0.0f;

  float m_angle_x = 0;

  float m_angle_y = 0;

  glm::vec3 m_position = glm::vec3(0, 0, 0);
};

} // namespace

class AppBaseImpl final
{
  friend AppBase;

  AppBaseImpl(GLFWwindow* window)
    : m_camera(new FirstPersonCamera())
  {
    setup_shader_program();

    setup_buffers();

    setup_textures();

    glUseProgram(m_program);

    m_pos_attr_location = glGetAttribLocation(m_program, "g_pos");

    glEnableVertexAttribArray(m_pos_attr_location);

    glVertexAttribPointer(
      m_pos_attr_location, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);

    int w = 0;
    int h = 0;
    glfwGetWindowSize(window, &w, &h);

    m_frame.resize(w, h);
  }

  ~AppBaseImpl()
  {
    glDeleteBuffers(1, &m_vertex_buffer);

    glDeleteTextures(1, &m_texture);

    glDeleteProgram(m_program);
  }

  void on_frame(AppBase& app)
  {
    app.render_imgui();

    if (m_camera->is_moving()) {

      m_camera->move();

      app.on_camera_change();
    }

    app.render(&m_frame.color[0], m_frame.w, m_frame.h);

    glBindTexture(GL_TEXTURE_2D, m_texture);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 m_frame.w,
                 m_frame.h,
                 0,
                 GL_RGB,
                 GL_FLOAT,
                 &m_frame.color[0]);

    glUseProgram(m_program);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

  void on_close() {}

  bool frame_clicked() const noexcept { return m_frame_clicked; }

  void on_cursor_motion(double x, double y, AppBase& app)
  {
    if (!m_frame_clicked)
      return;

    if (!m_frame_pos_initialized) {

      m_last_frame_pos = glm::vec2(x, y);

      m_frame_pos_initialized = true;
    }

    float dx = x - m_last_frame_pos.x;
    float dy = y - m_last_frame_pos.y;

    m_last_frame_pos.x = x;
    m_last_frame_pos.y = y;

    app.on_cursor_motion(x, y, dx, dy);
  }

  void on_cursor_motion(double /* x */, double /* y */, double dx, double dy)
  {
    m_camera->handle_relative_motion(dx, dy);
  }

  void on_cursor_button(int button, int action, int /* mods */)
  {
    if (ImGui::GetIO().WantCaptureMouse)
      return;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {

      m_frame_clicked = (action == GLFW_PRESS);

      if (m_frame_clicked)
        m_frame_pos_initialized = false;
    }
  }

  void on_key(int key, int /* scancode */, int action, int /* mods */)
  {
    m_camera->handle_key(key, action);
  }

  void on_resize(int w, int h) { m_frame.resize(w, h); }

  glm::vec3 get_camera_position() const { return m_camera->get_position(); }

  glm::mat3 get_camera_rotation_transform() const
  {
    return m_camera->get_rotation_transform();
  }

private:
  bool setup_shader_program()
  {
    auto vert_shader =
      compile_shader(GL_VERTEX_SHADER, g_vert_shader, std::cerr);

    if (!vert_shader)
      return false;

    auto frag_shader =
      compile_shader(GL_FRAGMENT_SHADER, g_frag_shader, std::cerr);

    if (!frag_shader) {
      glDeleteShader(vert_shader);
      return false;
    }

    m_program = link_shader_program(vert_shader, frag_shader, std::cerr);

    glDeleteShader(vert_shader);

    glDeleteShader(frag_shader);

    return m_program != 0;
  }

  void setup_buffers()
  {
    glGenBuffers(1, &m_vertex_buffer);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);

    float vertices[8];
    vertices[0] = -1; // top left
    vertices[1] = 1;
    vertices[2] = -1; // bottom left
    vertices[3] = -1;
    vertices[4] = 1; // top right
    vertices[5] = 1;
    vertices[6] = 1; // bottom right
    vertices[7] = -1;

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  }

  void setup_textures()
  {
    glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &m_texture);

    glBindTexture(GL_TEXTURE_2D, m_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // clang-format off
    float initColorBuf[3 * 4] {
      1, 0, 0,  0, 1, 0,
      0, 0, 1,  1, 1, 1
    };
    // clang-format on

    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, initColorBuf);
  }

  bool save_png() const
  {
    auto path = get_png_path();

    if (path.empty())
      return false;

    return save_png(path.c_str());
  }

  bool save_png(const char* path) const
  {
    using byte = unsigned char;

    std::vector<byte> data(m_frame.w * m_frame.h * 3);

    for (int i = 0; i < data.size(); i++)
      data[i] = std::max(std::min(m_frame.color[i] * 255.0f, 255.0f), 0.0f);

    int pitch = m_frame.w * 3;

    return stbi_write_png(path, m_frame.w, m_frame.h, 3, &data[0], pitch);
  }

  static std::string get_png_path()
  {
    for (int i = 0; i < 1024; i++) {

      std::ostringstream stream;

      stream << "snapshot-";
      stream << std::setfill('0') << std::setw(4) << i;
      stream << ".png";

      auto path = stream.str();

      if (!file_exists(path))
        return path;
    }

    return "";
  }

  static bool file_exists(const std::string& path)
  {
    std::ifstream file(path.c_str());

    return file.good();
  }

private:
  GLuint m_vertex_buffer = 0;

  GLuint m_texture = 0;

  GLuint m_program = 0;

  GLint m_pos_attr_location = 0;

  Frame m_frame;

  std::unique_ptr<Camera> m_camera;

  bool m_frame_clicked = false;

  bool m_frame_pos_initialized = false;

  glm::vec2 m_last_frame_pos = glm::vec2(0, 0);
};

AppBase::AppBase(GLFWwindow* window)
  : App(window)
  , m_impl(new AppBaseImpl(window))
{}

AppBase::~AppBase()
{
  delete m_impl;
}

void
AppBase::on_frame()
{
  m_impl->on_frame(*this);
}

void
AppBase::on_key(int key, int scancode, int action, int mods)
{
  m_impl->on_key(key, scancode, action, mods);
}

void
AppBase::on_resize(int w, int h)
{
  return m_impl->on_resize(w, h);
}

void
AppBase::on_close()
{}

void
AppBase::on_camera_change()
{}

glm::vec3
AppBase::get_camera_position() const
{
  return m_impl->get_camera_position();
}

glm::mat3
AppBase::get_camera_rotation_transform() const
{
  return m_impl->get_camera_rotation_transform();
}

void
AppBase::on_cursor_motion(double x, double y)
{
  if (!m_impl->frame_clicked())
    return;

  int xMax = 0;
  int yMax = 0;
  glfwGetWindowSize(get_glfw_window(), &xMax, &yMax);

  m_impl->on_cursor_motion(x / xMax, y / yMax, *this);

  on_camera_change();
}

void
AppBase::on_cursor_button(int button, int action, int mods)
{
  m_impl->on_cursor_button(button, action, mods);
}

void
AppBase::on_cursor_motion(double x, double y, double dx, double dy)
{
  m_impl->on_cursor_motion(x, y, dx, dy);
}

void
AppBase::render_imgui()
{
  ImGui::Begin("Control");

  ImGui::Text("FPS = %.1f", ImGui::GetIO().Framerate);

  if (ImGui::Button("Save PNG"))
    m_impl->save_png();

  if (ImGui::Button("Quit"))
    glfwSetWindowShouldClose(get_glfw_window(), true);

  ImGui::End();
}

} // namespace window_blit
