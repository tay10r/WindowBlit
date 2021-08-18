WindowBlit
==========

This project is for making it easy to debug path tracers or software renderers.
It was written so that there is something better to offer than rendering to a PPM
file and then opening the image to view the result.

This library contains all the boilerplate needed to visualize the results of a
path tracer at each iteration, while requiring very little boilerplate code.

Here is an example:

```cpp
#include <btn/btn.h>

class Example final : public btn::RtApp
{
public:
  using btn::RtApp::RtApp;

  void render(float* rgb_buffer, int w, int h) override
  {
    /* 'rgb_buffer' is copied to the window after this function. */
  }
};

int
main()
{
  return btn::run_glfw_window(btn::AppFactory<Example>());
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
 - On Windows, there is no program to open Netpbm files, so there's no need to install other software.

### A More Complete Example

In a window environment, things can get a little more involved because the
window can be resized and the camera can be moved. While the example above is
sufficient to get started, you will eventually want to overload other virtual
functions in the class. Here's a more complete example.

```cxx
#include <btn/btn.h>

class Example final : public btn::RtApp
{
public:
  using btn::RtApp::RtApp;

  void render(float* rgb_buffer, int w, int h) override
  {
    /* 'rgb_buffer' is copied to the window after this function. */

    /* Hint: Use 'get_camera_position()' and 'get_camera_rotation_transform()'
             to apply camera motion to the window. */
  }

  void on_resize(int w, int h) override
  {
    /* Override this function if needed. If storing a seperate RGB buffer,
       it should be resized here. For path tracers, this will usually require
       restarting the current frame. */
  }

  void on_camera_change() override
  {
    /* for path tracers, this will usually require restarting the current frame. */
  }
};

int
main()
{
  return btn::run_glfw_window(btn::AppFactory<Example>());
}
```

### Building the Examples

By default, the examples are not built. To build them, pass the following option
when configuring the CMake build.

```cmake
cmake -DBTN_EXAMPLES=ON
```

### Portability

The code works on Linux and Windows, on any platform that supports OpenGL 3.0
and greater. Note that VirtualBox does not support OpenGL 3.0 (as of this
writing), so this code will not work on VirtualBox. VMware workstation player
does support OpenGL 3.0, if you need to be using a virtual machine.

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

### Integration

The easiest way to integrate this project is to use CMake's `FetchContent`
module.

```cmake
# Note that this requires CMake 3.14.7 and above. CMake 3.14.7 was released on
# September 30th, 2019.
include(FetchContent)
FetchContent_Declare(better_than_netpbm URL "https://github.com/tay10r/BetterThanNetpbm/archive/refs/heads/main.zip")
FetchContent_MakeAvailable(better_than_netpbm)
```

You can also add it as a Git submodule and then just include it in your cmake
build like this:

```cmake
add_subdirectory(path/to/better_than_netpbm)
```

Once either of the two steps above have been done, just link to the library.

```cmake
target_link_libraries(my_path_tracer PRIVATE better_than_netpbm)
```

Note that with this library comes GLM. If you're starting a path tracer from
scratch, you can leverage GLM by either linking to `better_than_netpbm` or by
linking to the `glm` target.
