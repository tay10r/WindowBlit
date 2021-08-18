#pragma once

#include <glad/glad.h>

#include <iosfwd>
#include <string>

namespace window_blit {

/// Attempts to compile a shader.
///
/// @return On success, the ID of the shader is returned.
/// On failure, the value of zero is returned.
GLuint
compile_shader(GLenum shader_type,
               const std::string& source,
               std::ostream& errlog);

/// Links a simple shader program, consisting of a vertex and fragment shader.
///
/// @return On success, the ID of the shader program is returned.
/// On failure, the value of zero is returned instead.
GLuint
link_shader_program(GLuint vert_shader,
                    GLuint frag_shader,
                    std::ostream& errlog);

} // namespace window_blit
