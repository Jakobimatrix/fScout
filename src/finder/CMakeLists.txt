add_library(finder_lib STATIC
  src/finder/Dictionary.h
  src/finder/Dictionary.cpp
  src/finder/Needle.h
  src/finder/Needle.cpp
  src/finder/Tree.h
  src/finder/Tree.cpp
  src/finder/TreeNode.h
  src/finder/TreeNode.cpp
  src/finder/Finder.h
  src/finder/Finder.cpp
  src/finder/SearchPattern.h
  )

target_link_libraries(finder_lib
  settings_lib_1.0.0
  utils_lib_0.0.1
  globals_lib
  timer_lib
  )

target_include_directories(finder_lib PUBLIC STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src")


