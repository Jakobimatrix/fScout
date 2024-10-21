add_library(display_lib STATIC
  src/display/HoverableListWidget.cpp
  src/display/finderWidget.cpp
  src/display/finderOutputWidget.cpp
  src/display/display.cpp
  src/display/displayQt.cpp
  )

target_link_libraries(display_lib
  Qt5::Widgets
  settings_lib_1.0.0
  globals_lib
  finder_lib
  )

target_include_directories(display_lib PUBLIC STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
