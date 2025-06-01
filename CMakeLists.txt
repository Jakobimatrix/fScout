cmake_minimum_required(VERSION 3.16)

# Define project if not already defined (useful for subproject inclusion)
if (NOT DEFINED PROJECT_NAME)
    project(fScout LANGUAGES CXX)

    # This variable must exist!
    set(ENVIRONMENT_SETTINGS "ProjectSettings")

    add_library("${ENVIRONMENT_SETTINGS}" INTERFACE)

    # ===== Compiler Settings =====
    include(initRepo/cmake/CompilerSetup.cmake)

    # ===== Warnings and Tools =====
    include(initRepo/cmake/CompilerWarnings.cmake)
    set_project_warnings(${ENVIRONMENT_SETTINGS})

    # ===== External Dependencies =====
    include(initRepo/cmake/Includes.cmake)

    # ===== Options =====
    include(initRepo/cmake/Options.cmake)
    enable_multithreading(${ENVIRONMENT_SETTINGS})
    enable_shared_libraries(${ENVIRONMENT_SETTINGS})

    # ===== Fuzzing =====
    include(initRepo/cmake/ClangFuzzyTests.cmake)

    # ===== CTest with Catch2 =====
    setup_catch2_and_ctest()
    enable_testing()
endif()

# supress console on release mode for windows
if (WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # For MinGW (GCC on Windows)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-subsystem,windows")
  else()
    # For MSVC
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
  endif()
endif()



# Auto-run Qt's meta-object compiler (moc), resource compiler (rcc), and user interface compiler (uic)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)


set(CMAKE_CXX_STANDARD 20)

if(WIN32)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_PREFIX_PATH "C:/Qt/Tools/mingw1310_64" CACHE PATH "Qt5 MinGW Path")
  else()
  message(FATAL_ERROR "Compiler not supported")
  endif()
endif()

add_subdirectory(src/timer)
add_subdirectory(src/settings)
add_subdirectory(src/globals)
add_subdirectory(src/finder)
add_subdirectory(src/display)
add_subdirectory(src/executables)
