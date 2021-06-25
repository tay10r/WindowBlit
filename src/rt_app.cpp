#include <btn/rt_app.h>

#include "shader.h"

#include "stb_image_write.h"

#include <glm/glm.hpp>

#include <imgui.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include <cstdint>

namespace btn {

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

  virtual bool is_moving() const = 0;

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

    m_position += delta * m_move_speed;
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

  glm::vec3 get_position() const override { return m_position; }

  glm::mat3 get_rotation_transform() const override { return glm::mat3(1.0f); }

private:
  float m_move_speed = 0.05;

  float m_up_speed = 0.0f;

  float m_left_speed = 0.0f;

  float m_down_speed = 0.0f;

  float m_right_speed = 0.0f;

  glm::vec3 m_position = glm::vec3(0, 0, 0);
};

} // namespace

class RtAppImpl final
{
public:
  RtAppImpl(GLFWwindow* window)
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

  ~RtAppImpl()
  {
    glDeleteBuffers(1, &m_vertex_buffer);

    glDeleteTextures(1, &m_texture);

    glDeleteProgram(m_program);
  }

  void on_frame(RtApp& rt_app)
  {
    render_gui();

    if (m_camera->is_moving()) {

      m_camera->move();

      rt_app.on_camera_change();
    }

    rt_app.render(&m_frame.color[0], m_frame.w, m_frame.h);

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
  void render_gui()
  {
    ImGui::Begin("Control");

    ImGui::Text("FPS = %.1f", ImGui::GetIO().Framerate);

    if (ImGui::Button("Save PNG"))
      save_png();

    ImGui::End();
  }

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
};

RtApp::RtApp(GLFWwindow* window)
  : App(window)
  , m_impl(new RtAppImpl(window))
{}

RtApp::~RtApp()
{
  delete m_impl;
}

void
RtApp::on_frame()
{
  m_impl->on_frame(*this);
}

void
RtApp::on_key(int key, int scancode, int action, int mods)
{
  m_impl->on_key(key, scancode, action, mods);
}

void
RtApp::on_resize(int w, int h)
{
  return m_impl->on_resize(w, h);
}

void
RtApp::on_close()
{}

void
RtApp::on_camera_change()
{}

glm::vec3
RtApp::get_camera_position() const
{
  return m_impl->get_camera_position();
}

glm::mat3
RtApp::get_camera_rotation_transform() const
{
  return m_impl->get_camera_rotation_transform();
}

} // namespace btn
