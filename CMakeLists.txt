cmake_minimum_required(VERSION 3.20)
project(RayTracerModern CXX)

# C++23 (GCC 13 supports -std=c++23). The original code was pre-C++17;
# see the "C++23" commit for the one removed-feature fix this required.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(OpenMP REQUIRED)
find_package(glm REQUIRED)
# glfw / imgui / stb are pulled in by the executable target below.

# --- Render core (Qt-free, unchanged from the original apart from the
# --- RayTracer API: configurable samples / scene, see raytracer.h) ---
add_library(raytracer_core STATIC
    ray.cpp
    raytracer.cpp
    sphere.cpp
    hitablelist.cpp
    camera.cpp
    lambertian.cpp
    utils.cpp
    metal.cpp
    dielectric.cpp
    movingsphere.cpp
    aabbox.cpp
    bvh.cpp
    constanttexture.cpp
    checkertexture.cpp
    perlin.cpp
    noisetexture.cpp
    diffuselight.cpp
    material.cpp
    xyrect.cpp
    xzrect.cpp
    yzrect.cpp
    flipnormals.cpp
    aabox.cpp
)
target_link_libraries(raytracer_core PUBLIC glm::glm OpenMP::OpenMP_CXX)
target_compile_options(raytracer_core PRIVATE -O2 -Wall -Wextra)

# --- Executable: GLFW + Dear ImGui GUI, plus --headless CLI mode ---
# (added in the "GLFW + ImGui GUI" commit; main.cpp does not exist yet
# in the scaffolding commit, so this target is added there.)
