#include <window_blit/window_blit.hpp>

namespace {

class MinimalExample final : public window_blit::AppBase
{
public:
  using window_blit::AppBase::AppBase;

  void render(float* rgb_buffer, int w, int h) override
  {
    for (int i = 0; i < (w * h); i++) {

      int x = i % w;
      int y = i / w;

      float u = (x + 0.5f) / w;
      float v = (y + 0.5f) / h;

      rgb_buffer[(i * 3) + 0] = u;
      rgb_buffer[(i * 3) + 1] = v;
      rgb_buffer[(i * 3) + 2] = 1;
    }
  }
};

} // namespace

int
main()
{
  return window_blit::run_glfw_window(
    window_blit::AppFactory<MinimalExample>());
}
