cmake_minimum_required(VERSION 3.9.6)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

add_library(glad STATIC src/glad.c)

target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(WIN32)
  target_link_libraries(glad PUBLIC ${OPENGL_LIBRARIES})
else(WIN32)
  target_link_libraries(glad PUBLIC OpenGL::OpenGL)
endif(WIN32)
