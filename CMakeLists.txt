cmake_minimum_required(VERSION 3.14.7)

project(open_gl_2_template)

include(FetchContent)

FetchContent_Declare(glfw
  URL "https://github.com/glfw/glfw/archive/master.zip"
  URL_HASH "SHA256=b7f72f7098e1418657cb95b296baba32543f0996c93ebe3f6e8dfca7374ebab2")

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
