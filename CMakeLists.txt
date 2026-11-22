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
find_package(glfw3 REQUIRED)
find_package(imgui REQUIRED)
find_package(stb REQUIRED)
find_package(OpenGL REQUIRED)

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
# The Conan imgui package ships the GLFW/OpenGL3 backends under
# res/bindings/ (not on the default include path), so locate them
# relative to the imgui target's include directory and compile them in.
# INTERFACE_INCLUDE_DIRECTORIES arrives as a generator expression
# ($<$<CONFIG:Release>:<pkg>/include>), so pull the real path out.
get_target_property(_imgui_include_dirs imgui::imgui INTERFACE_INCLUDE_DIRECTORIES)
string(REGEX MATCH "/[^$>]*/p/include" _imgui_include_dir "${_imgui_include_dirs}")
if(NOT _imgui_include_dir)
    message(FATAL_ERROR "Could not locate imgui package include dir in: ${_imgui_include_dirs}")
endif()
get_filename_component(_imgui_pkg_dir "${_imgui_include_dir}" DIRECTORY)
set(IMGUI_BACKEND_DIR "${_imgui_pkg_dir}/res/bindings")
if(NOT EXISTS "${IMGUI_BACKEND_DIR}/imgui_impl_glfw.cpp")
    message(FATAL_ERROR "ImGui backends not found in Conan package at ${IMGUI_BACKEND_DIR}")
endif()

add_executable(RayTracerModern
    main.cpp
    "${IMGUI_BACKEND_DIR}/imgui_impl_glfw.cpp"
    "${IMGUI_BACKEND_DIR}/imgui_impl_opengl3.cpp"
)
target_include_directories(RayTracerModern PRIVATE "${IMGUI_BACKEND_DIR}")
target_link_libraries(RayTracerModern PRIVATE
    raytracer_core
    glfw  # Conan's glfw3 config declares the target un-namespaced
    imgui::imgui
    stb::stb
    OpenGL::GL
)
target_compile_options(RayTracerModern PRIVATE -O2 -Wall -Wextra)
