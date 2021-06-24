#include "glfw.h"
#include "rt_app.h"

namespace {

class ExampleApp final : public RtApp
{
public:
  using RtApp::RtApp;

  void render(float* rgb_buffer, int w, int h) override;
};

void
ExampleApp::render(float* rgb_buffer, int w, int h)
{
  for (int i = 0; i < (w * h); i++) {

    int x = i % w;
    int y = i / w;

    float u = (x + 0.5) / w;
    float v = (y + 0.5) / h;

    rgb_buffer[(i * 3) + 0] = u;
    rgb_buffer[(i * 3) + 1] = v;
    rgb_buffer[(i * 3) + 2] = 1;
  }
}

} // namespace

int
#ifdef _WIN32
  WINAPI
  wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
#else
main()
#endif
{
  return run_glfw_window(AppFactory<ExampleApp>());
}
