#pragma once

#ifndef BTN_GLFW_H_INCLUDED
#define BTN_GLFW_H_INCLUDED

namespace btn {

class AppFactoryBase;

int
run_glfw_window(const AppFactoryBase& app_factory);

} // namespace btn

#endif // BTN_GLFW_H_INCLUDED
