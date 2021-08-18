#pragma once

#ifndef WINDOW_BLIT_GLFW_HPP_INCLUDED
#define WINDOW_BLIT_GLFW_HPP_INCLUDED

namespace window_blit {

class AppFactoryBase;

int
run_glfw_window(const AppFactoryBase& app_factory);

} // namespace window_blit

#endif // WINDOW_BLIT_GLFW_HPP_INCLUDED
