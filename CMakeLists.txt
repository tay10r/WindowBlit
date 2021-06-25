cmake_minimum_required(VERSION 3.14.7)

project(better_than_netpbm)

add_subdirectory(glad)

include(FetchContent)

##############
# Setup GLFW #
##############

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")

FetchContent_Declare(glfw
  URL "https://github.com/glfw/glfw/archive/refs/tags/3.3.4.zip"
  URL_HASH "SHA256=19a1048439a35e49f9b48fbe2e42787cfabae70df80ffd096b3b553bbd8a09f7")

FetchContent_MakeAvailable(glfw)

###############
# Setup ImGui #
###############

FetchContent_Declare(imgui
  URL "https://github.com/ocornut/imgui/archive/refs/tags/v1.83.zip"
  URL_HASH "SHA256=56db0e98f4b828396391366b694049d38478562b98b8e29f6cef3b56cd8029d5")

FetchContent_MakeAvailable(imgui)

FetchContent_GetProperties(imgui)

set(btn_imgui_sources
  imconfig.h
  imgui.cpp
  imgui_demo.cpp
  imgui_draw.cpp
  imgui.h
  imgui_internal.h
  imgui_tables.cpp
  imgui_widgets.cpp
  imstb_rectpack.h
  imstb_textedit.h
  imstb_truetype.h
  backends/imgui_impl_glfw.cpp
  backends/imgui_impl_glfw.h
  backends/imgui_impl_opengl3.cpp
  backends/imgui_impl_opengl3.h)

list(TRANSFORM btn_imgui_sources PREPEND "${imgui_SOURCE_DIR}/")

add_library(btn_imgui ${btn_imgui_sources})

target_compile_definitions(btn_imgui
  PRIVATE
    IMGUI_IMPL_OPENGL_LOADER_GLAD=1)

target_include_directories(btn_imgui
  PUBLIC
    "${imgui_SOURCE_DIR}"
    "${imgui_SOURCE_DIR}/backends")

target_link_libraries(btn_imgui PUBLIC glad glfw)

#################
# Build Program #
#################

add_executable(main WIN32
  app.h
  app.cpp
  glfw.h
  glfw.cpp
  shader.h
  shader.cpp
  rt_app.h
  rt_app.cpp
  main.cpp)

target_link_libraries(main PRIVATE glfw glad btn_imgui)
