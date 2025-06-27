cmake_minimum_required(VERSION 3.10)
project(raylib_test C)

# Require C11 
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Allow override of install location
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
    GIT_TAG        v4.5.0
  )
  # disable examples/tests inside raylib
  set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(raylib)
endif()

# Your executable
add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# After each build, copy the entire resources/ tree next to the exe
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)
