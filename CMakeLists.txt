cmake_minimum_required(VERSION 3.10)
project(raylib_test CXX)

# Set C++ standard
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Require C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Default to Debug build for debug symbols if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Add debug symbols to compiler and linker flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g")

# Allow override of install location via RAYLIB_ROOT environment variable
if(DEFINED ENV{RAYLIB_ROOT})
  list(APPEND CMAKE_PREFIX_PATH $ENV{RAYLIB_ROOT})
endif()

# Try Config-mode find_package
find_package(raylib CONFIG QUIET)

# Try pkg-config if Config failed
if(NOT raylib_FOUND)
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(RAYLIB_PKG raylib)
    if(RAYLIB_PKG_FOUND)
      add_library(raylib INTERFACE)
      target_include_directories(raylib INTERFACE ${RAYLIB_PKG_INCLUDE_DIRS})
      target_link_libraries(raylib INTERFACE ${RAYLIB_PKG_LIBRARIES})
      set(raylib_FOUND TRUE)
    endif()
  endif()
endif()

# Fetch & build raylib if still missing
if(NOT raylib_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        master   # use main branch to avoid invalid tag reference
  )
  # Disable examples/tests inside raylib
  set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(raylib)
endif()

# Use FetchContent to download and build dependencies
include(FetchContent)

# Add ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.9b
)
FetchContent_MakeAvailable(imgui)

# Add rlImGui
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG main  # Use main branch as it's actively maintained
)
FetchContent_MakeAvailable(rlimgui)

# Create rlImGui sources
set(RLIMGUI_SOURCES
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
    ${rlimgui_SOURCE_DIR}/rlImGui.h
)

# Create ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Add executable
add_executable(${PROJECT_NAME} main.cpp ${IMGUI_SOURCES} ${RLIMGUI_SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Add include directories for ImGui and rlImGui
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
)

# Place the .exe in the top of the build tree
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# After each build, copy the entire resources/ tree next to the exe
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        "${CMAKE_BINARY_DIR}/resources" 
)
