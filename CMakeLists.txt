cmake_minimum_required(VERSION 3.14.7)

project(window_blit)

option(WINDOWBLIT_EXAMPLES "Whether or not to build the examples." OFF)

option(WINDOWBLIT_FETCH_DEPS "Whether or not to manually download the examples." OFF)

option(WINDOWBLIT_DISABLE_IMGUI "Whether or not to disable ImGui." OFF)

add_subdirectory(glad)

include(FetchContent)

#############
# Setup GLM #
#############

if(WINDOWBLIT_FETCH_DEPS)

  FetchContent_Declare(glm
    URL "https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.zip"
    URL_HASH "SHA256=4605259c22feadf35388c027f07b345ad3aa3b12631a5a316347f7566c6f1839")

  FetchContent_MakeAvailable(glm)

else(WINDOWBLIT_FETCH_DEPS)

  find_package(glm REQUIRED)

endif(WINDOWBLIT_FETCH_DEPS)

##############
# Setup GLFW #
##############

if(WINDOWBLIT_FETCH_DEPS)

  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "")

  FetchContent_Declare(glfw
    URL "https://github.com/glfw/glfw/archive/refs/tags/3.3.4.zip"
    URL_HASH "SHA256=19a1048439a35e49f9b48fbe2e42787cfabae70df80ffd096b3b553bbd8a09f7")

  FetchContent_MakeAvailable(glfw)

else(WINDOWBLIT_FETCH_DEPS)

  find_package(glfw3 REQUIRED)

endif(WINDOWBLIT_FETCH_DEPS)


###############
# Setup ImGui #
###############

if(NOT WINDOWBLIT_DISABLE_IMGUI)

  FetchContent_Declare(imgui
    URL "https://github.com/ocornut/imgui/archive/refs/tags/v1.83.zip"
    URL_HASH "SHA256=56db0e98f4b828396391366b694049d38478562b98b8e29f6cef3b56cd8029d5")

  FetchContent_MakeAvailable(imgui)

  FetchContent_GetProperties(imgui)

  set(windowblit_imgui_sources
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

  list(TRANSFORM windowblit_imgui_sources PREPEND "${imgui_SOURCE_DIR}/")

  add_library(windowblit_imgui ${windowblit_imgui_sources})

  target_compile_definitions(windowblit_imgui
    PRIVATE
      IMGUI_IMPL_OPENGL_LOADER_GLAD=1)

  target_include_directories(windowblit_imgui
    PUBLIC
      "${imgui_SOURCE_DIR}"
      "${imgui_SOURCE_DIR}/backends")

  target_link_libraries(windowblit_imgui PUBLIC glad glfw)

endif(NOT WINDOWBLIT_DISABLE_IMGUI)

#################
# Build Library #
#################

add_library(window_blit
  include/window_blit/app.hpp
  include/window_blit/app_base.hpp
  include/window_blit/glfw.hpp
  src/app.cpp
  src/app_base.cpp
  src/glfw.cpp
  src/shader.hpp
  src/shader.cpp
  src/stb_image_write.h
  src/stb_image_write.c)

if(WINDOWBLIT_DISABLE_IMGUI)
  target_compile_definitions(window_blit PRIVATE WINDOWBLIT_DISABLE_IMGUI=1)
endif(WINDOWBLIT_DISABLE_IMGUI)

target_include_directories(window_blit PUBLIC "${PROJECT_SOURCE_DIR}/include")

target_link_libraries(window_blit PUBLIC glfw glad)

if(NOT WINDOWBLIT_DISABLE_IMGUI)
  target_link_libraries(window_blit PUBLIC windowblit_imgui)
endif(NOT WINDOWBLIT_DISABLE_IMGUI)

if(UNIX)
  target_link_libraries(window_blit PUBLIC dl)
endif(UNIX)

if(TARGET glm)
  target_link_libraries(window_blit PUBLIC glm)
elseif(TARGET glm::glm)
  target_link_libraries(window_blit PUBLIC glm::glm)
endif(TARGET glm)

##################
# Build Examples #
##################

if(WINDOWBLIT_EXAMPLES)

  find_package(OpenMP)

  set(examples
    minimal
    path_tracer
    planets)

  foreach(example ${examples})

    set(target window_blit_example_${example})

    add_executable(${target} WIN32 examples/${example}/main.cpp)

    target_link_libraries(${target} PRIVATE window_blit)

    if(OpenMP_FOUND)
      target_link_libraries(${target} PRIVATE OpenMP::OpenMP_CXX)
    endif(OpenMP_FOUND)

    set_target_properties(${target}
      PROPERTIES
        OUTPUT_NAME ${example})

  endforeach(example ${examples})

endif(WINDOWBLIT_EXAMPLES)
