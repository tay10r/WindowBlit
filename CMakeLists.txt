cmake_minimum_required(VERSION 3.14.7)

project(open_gl_2_template)

include(FetchContent)

# Setup GLFW

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")

FetchContent_Declare(glfw
  URL "https://github.com/glfw/glfw/archive/refs/tags/3.3.4.zip"
  URL_HASH "SHA256=19a1048439a35e49f9b48fbe2e42787cfabae70df80ffd096b3b553bbd8a09f7")

FetchContent_MakeAvailable(glfw)

add_subdirectory(glad)

add_executable(main
  app.h
  app.cpp
  glfw.h
  glfw.cpp
  shader.h
  shader.cpp
  rt_app.h
  rt_app.cpp
  main.cpp)

target_link_libraries(main PRIVATE glfw glad)
