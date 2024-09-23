cmake_minimum_required(VERSION 3.15)

# Set the project name
project(finder VERSION 1.0.0)

# Include custom CMake modules
include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

# Set C++ standard and compile-time options
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -pthread")

# Time trace options for Clang (Linux builds)
if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()
endif()

# Standard compiler warnings and static analysis options
add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)
include(cmake/StaticAnalyzers.cmake)
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# Doxygen documentation support
include(cmake/Doxygen.cmake)
enable_doxygen()


# Set build type (Debug or Release)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message("############### BUILD TYPE: ${CMAKE_BUILD_TYPE} ###############")

# Auto-run Qt's meta-object compiler (moc), resource compiler (rcc), and user interface compiler (uic)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find and link Qt5 libraries
find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)
find_package(Qt5OpenGL REQUIRED)

# Set required C++ standard (in this case, C++20)
set(CMAKE_CXX_STANDARD 20)

# Find the Qt5 libraries (path to MinGW compiled Qt libs should be set here for cross-compiling)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  set(CMAKE_PREFIX_PATH "/usr/x86_64-w64-mingw32/lib/cmake/Qt5")  # Update this to your correct MinGW Qt path
endif()

# Add submodules for project organization
add_subdirectory(src/settings)
add_subdirectory(src/globals)
add_subdirectory(src/finder)
add_subdirectory(src/display)

# Add the main executable
add_subdirectory(src/executable)

