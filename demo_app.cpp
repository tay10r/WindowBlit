#include "app.h"

#include "shader.h"

#include <iostream>

#include <cstdint>

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

class DemoApp final : public App
{
public:
  DemoApp(GLFWwindow* window)
    : App(window)
  {
    setup_shader_program();

    setup_buffers();

    setup_textures();

    glUseProgram(m_program);

    m_pos_attr_location = glGetAttribLocation(m_program, "g_pos");

    glEnableVertexAttribArray(m_pos_attr_location);

    glVertexAttribPointer(
      m_pos_attr_location, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
  }

  void on_frame() override
  {
    glUseProgram(m_program);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, m_texture);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

  void on_close() override {}

  void on_key(int key, int scancode, int action, int mods) override
  {
    (void)key;
    (void)scancode;
    (void)action;
    (void)mods;
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

private:
  GLuint m_vertex_buffer = 0;

  GLuint m_texture = 0;

  GLuint m_program = 0;

  GLint m_pos_attr_location = 0;
};

} // namespace

App*
App::create_app(GLFWwindow* window)
{
  return new DemoApp(window);
}
