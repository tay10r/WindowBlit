cmake_minimum_required(VERSION 3.14.7)

project(better_than_netpbm)

option(BTN_EXAMPLES "Whether or not to build the examples." OFF)

add_subdirectory(glad)

include(FetchContent)

#############
# Setup GLM #
#############

FetchContent_Declare(glm
  URL "https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.zip"
  URL_HASH "SHA256=4605259c22feadf35388c027f07b345ad3aa3b12631a5a316347f7566c6f1839")

FetchContent_MakeAvailable(glm)

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
# Build Library #
#################

add_library(better_than_netpbm
  include/btn/app.h
  include/btn/glfw.h
  include/btn/rt_app.h
  src/app.cpp
  src/glfw.cpp
  src/shader.h
  src/shader.cpp
  src/rt_app.cpp
  src/stb_image_write.h
  src/stb_image_write.c)

target_include_directories(better_than_netpbm PUBLIC "${PROJECT_SOURCE_DIR}/include")

target_link_libraries(better_than_netpbm PUBLIC glfw glad btn_imgui glm)

##################
# Build Examples #
##################

if(BTN_EXAMPLES)

  set(examples
    minimal
    path_tracer)

  foreach(example ${examples})

    set(target better_than_netpbm_example_${example})

    add_executable(${target} WIN32 examples/${example}/main.cpp)

    target_link_libraries(${target} better_than_netpbm)

    set_target_properties(${target}
      PROPERTIES
        OUTPUT_NAME ${example})

  endforeach(example ${examples})

endif(BTN_EXAMPLES)
