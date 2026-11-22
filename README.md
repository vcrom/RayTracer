## Basic Ray Tracer (modernised)

A simple Raytracer following the guidelines of the [Ray Tracing Minibooks](https://raytracing.github.io/) by Peter Shirley.

This is an initial version where I implemented the techniques described in the first book of the series and some of the second.

### Build (modernised: CMake + Conan, C++23, GLFW + Dear ImGui)

The original Qt 5 / qmake build was replaced (see git history — the
import commit preserves it). Dependencies are managed by Conan 2:

```sh
# once: pip install conan   (Conan 2.x)
conan install . --build=missing
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/RayTracerModern
```

Requires a C++23 compiler (GCC 13+, `-std=c++23`), CMake 3.20+,
OpenMP, and on Linux the X11/GL development packages GLFW needs
(`libgl1-mesa-dev`, `libx11-dev`, `libxrandr-dev`, `libxinerama-dev`,
`libxcursor-dev`, `libxi-dev`, ... — `conan install` will list any
that are missing).

### Usage

GUI: pick resolution, samples per pixel (default 1000, the original
hardcoded value; 1–10 for previews, 100 for drafts), and a scene —
Cornell box, Book cover, Basic scene or Simple scene — then Render.
Rendering runs on a worker thread with a progress bar; Save PNG writes
the image via stb_image_write.

Headless (no display needed):

```sh
./build/RayTracerModern --headless --width 256 --height 256 --samples 100 --scene cornell --output cornell.png
```

Scenes: `cornell`, `bookcover`, `basic`, `simple`. Defaults:
256×256, 100 samples, `cornell`, `out.png`.
