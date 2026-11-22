# Conan 2 dependency provider, in the spirit of vcrom/OpenGlApp: that
# project downloads conan.cmake at configure time and runs
# conan_cmake_run() itself. The Conan 1 helper (and the bincrafters
# remote) are obsolete, so the modern equivalent is the official
# cmake-conan provider (conan-io/cmake-conan, develop2 branch):
# CMake calls it from find_package() when a package is missing, and
# it runs `conan install` on the conanfile.txt in this directory,
# then retries the find_package() call.
# No manual `conan install` and no -DCMAKE_TOOLCHAIN_FILE are needed:
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build -j
# Passing a Conan toolchain file explicitly still works: the packages
# are then found directly and the provider never fires.
# Requires CMake 3.24+ (CMAKE_PROJECT_TOP_LEVEL_INCLUDES).
# Note: there is no cmake-conan2 repo — the provider lives in
# conan-io/cmake-conan on the develop2 branch.
cmake_minimum_required(VERSION 3.24)

if(NOT CMAKE_PROJECT_TOP_LEVEL_INCLUDES AND NOT CMAKE_TOOLCHAIN_FILE)
    find_program(CONAN_CMD conan)
    if(NOT CONAN_CMD)
        message(FATAL_ERROR "Conan executable not found in PATH; install Conan 2 "
                "(e.g. `pip install conan`) or configure with an explicit Conan "
                "toolchain file: -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake")
    endif()
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan_provider.cmake")
        message(STATUS "Downloading conan_provider.cmake from https://github.com/conan-io/cmake-conan (develop2)")
        file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/develop2/conan_provider.cmake"
             "${CMAKE_BINARY_DIR}/conan_provider.cmake"
             STATUS _conan_provider_dl_status)
        list(GET _conan_provider_dl_status 0 _conan_provider_dl_code)
        if(NOT _conan_provider_dl_code EQUAL 0)
            message(FATAL_ERROR "Failed to download conan_provider.cmake: ${_conan_provider_dl_status}")
        endif()
    endif()
    set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES "${CMAKE_BINARY_DIR}/conan_provider.cmake")
endif()

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
