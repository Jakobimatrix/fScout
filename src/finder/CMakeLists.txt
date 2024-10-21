add_library(finder_lib STATIC
  src/finder/Dictionary.cpp
  src/finder/Tree.cpp
  src/finder/TreeNode.cpp
  src/finder/Finder.cpp
  src/finder/SearchPattern.h
  )

target_link_libraries(finder_lib
  settings_lib_1.0.0
  globals_lib
  )

target_include_directories(finder_lib PUBLIC STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src")


