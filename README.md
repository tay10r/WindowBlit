BetterThanNetpbm
================

This project is for making it easy to debug path tracers. It was written so that
there is something better to offer than rendering to a PPM file and then opening
the image to view the result. More sophisticated path tracers will include a
window to preview the result, but this adds a lot of boilerplate to the project.
This library contains all the boilerplate needed to visualize the results of a
path tracer at each iteration, while requiring very little boilerplate code.

Here is an example:

```cpp
#include "rt_app.h"
#include "glfw.h"

class Example final : public RtApp
{
public:
  using RtApp::RtApp;

  void render(float* rgb_buffer, int w, int h) override
  {
    /* render one sampler per pixel, accumulate in 'rgb_buffer' */
  }
};

int
main()
{
  return run_glfw_window(AppFactory<Example>());
}
```

Compare it to the code required to write a PPM file.

```cpp
#include <fstream>
#include <algorithm>

int
main()
{
  /* render image */

  std::ofstream file("image.ppm", std::ios::binary | std::ios::out);

  file << "P6\n" << w << ' ' << h << std::endl << "255" << std::endl;

  std::vector<unsigned char> rgb_byte_buffer;

  for (int i = 0; i < (w * h); i++)
    rgb_byte_buffer[i] = std::min(std::max(rgb_buffer[i] * 255, 0.0f), 255.0f);

  file.write((const char*) rgb_byte_buffer.data(), rgb_byte_buffer.size());
}
```

Both are about the same number of lines, but the first will show you the results
a lot more quickly. This has the potential to increase developer productivity
because:

 - Results will show up faster (no need to wait for the whole render)
 - No need to enter a second command in order to open an image viewer
 - On Windows, there is no program to open Netpbm files, so this elimates that headache.

### Portability

The code works on Linux and Windows, on any platform that supports OpenGL 3.0
and greater. Note that VirtualBox does not support OpenGL 3.0 (as of this
writing), so this code will not work on VirtualBox.

### Dependencies

If you're building this on Ubuntu, you'll need the following libraries to build GLFW:

 - `libxrandr-dev`
 - `libxinerama-dev`
 - `libxcursor-dev`
 - `libxi-dev`

You can run the following command to install them all:

```
sudo apt install libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

All other dependencies are either included or downloaded and built.
