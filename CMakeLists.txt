cmake_minimum_required(VERSION 3.10)
project(raylib_test C)

# Require C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Tell CMake where Homebrew puts raylib
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/raylib")

# Find the raylib config file
find_package(raylib CONFIG REQUIRED)

# Your executable
add_executable(${PROJECT_NAME} main.c)

# Link in raylib (and its dependencies/frameworks)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
