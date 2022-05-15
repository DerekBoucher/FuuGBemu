cmake_minimum_required(VERSION 3.18.4)

project(FuuGBemu)

# Minimum C++ standard must be c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Check that the dependencies are installed
find_library(FOUND_PTHREAD pthread)
find_library(FOUND_GL GL)
find_library(FOUND_GLFW glfw)
find_library(FOUND_GLEW GLEW)

if (NOT FOUND_PTHREAD)
    message(FATAL_ERROR "pthread libraries not found. install via 'apt install libpthread-stubs0-dev'")
endif()

if (NOT FOUND_GL)
    message(FATAL_ERROR "opengl libraries not found. install via 'apt install libopengl-dev'")
endif()

if (NOT FOUND_GLFW)
    message(FATAL_ERROR "glfw libraries not found. install via 'apt install libglfw3-dev'")
endif()

if (NOT FOUND_GLEW)
    message(FATAL_ERROR "glfw libraries not found. install via 'apt install libglew-dev'")
endif()

# Detect all of the source files
file(GLOB sources src/*.cpp src/opengl/*cpp)

# Define the compilation output
add_executable(FuuGBemu "${sources}")

# Add include directory
target_include_directories(FuuGBemu PRIVATE "include")

# Link against external libraries
target_link_libraries(FuuGBemu PRIVATE GL glfw GLEW pthread)

# Enable all compiler warnings
add_compile_options(-Wall)

