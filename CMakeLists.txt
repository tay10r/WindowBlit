cmake_minimum_required(VERSION 3.14.7)

project(open_gl_2_template)

include(FetchContent)

FetchContent_Declare(glfw URL "https://github.com/glfw/glfw/archive/refs/tags/3.3.4.zip")

FetchContent_MakeAvailable(glfw)

add_subdirectory(glad)

add_executable(main
  app.h
  app.cpp
  shader.h
  shader.cpp
  demo_app.cpp
  main.cpp)

target_link_libraries(main PRIVATE glfw glad)
