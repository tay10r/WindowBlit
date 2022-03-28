#include <window_blit/window_blit.hpp>

#include <vector>

namespace {

class MinimalExample final : public window_blit::AppBase
{
public:
  using window_blit::AppBase::AppBase;

  void render(GLuint texture_id, int w, int h) override
  {
    std::vector<float> rgb(w * h * 3);

    for (int i = 0; i < (w * h); i++) {

      int x = i % w;
      int y = i / w;

      float u = (x + 0.5f) / w;
      float v = (y + 0.5f) / h;

      rgb[(i * 3) + 0] = u;
      rgb[(i * 3) + 1] = v;
      rgb[(i * 3) + 2] = 1;
    }

    load_rgb(&rgb[0], w, h, texture_id);
  }
};

} // namespace

int
main()
{
  return window_blit::run_glfw_window(
    window_blit::AppFactory<MinimalExample>());
}
