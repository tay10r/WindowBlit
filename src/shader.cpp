#include "shader.hpp"

#include <ostream>

namespace window_blit {

GLuint
compile_shader(GLenum shader_type,
               const std::string& source,
               std::ostream& errlog)
{
  GLuint id = glCreateShader(shader_type);
  if (id == 0)
    return id;

  const char* sourcePtr = source.c_str();

  GLint length = source.size();

  glShaderSource(id, 1, &sourcePtr, &length);

  glCompileShader(id);

  GLint success = GL_TRUE;

  glGetShaderiv(id, GL_COMPILE_STATUS, &success);

  if (success == GL_TRUE)
    return id;

  GLint log_size = 0;

  glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_size);

  std::string log;

  log.resize(log_size);

  glGetShaderInfoLog(id, log_size, &log_size, &log[0]);

  log.resize(log_size);

  glDeleteShader(id);

  errlog << log;

  return 0;
}

GLuint
link_shader_program(GLuint vert_shader,
                    GLuint frag_shader,
                    std::ostream& errlog)
{
  auto id = glCreateProgram();

  if (!id)
    return 0;

  glAttachShader(id, vert_shader);
  glAttachShader(id, frag_shader);

  glLinkProgram(id);

  glValidateProgram(id);

  glDetachShader(id, vert_shader);
  glDetachShader(id, frag_shader);

  GLint is_linked = GL_TRUE;

  glGetProgramiv(id, GL_LINK_STATUS, &is_linked);

  if (is_linked)
    return id;

  GLint log_length = 0;

  glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_length);

  std::string log;

  log.resize(log_length);

  glGetProgramInfoLog(id, log_length, &log_length, &log[0]);

  log.resize(log_length);

  glDeleteProgram(id);

  errlog << log;

  return 0;
}

} // namespace window_blit
