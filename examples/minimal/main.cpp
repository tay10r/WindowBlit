#include <btn/btn.h>

namespace {

class MinimalExample final : public btn::RtApp
{
public:
  using btn::RtApp::RtApp;

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
  return btn::run_glfw_window(btn::AppFactory<MinimalExample>());
  ;
}
