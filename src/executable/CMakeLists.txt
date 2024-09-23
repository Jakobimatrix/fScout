add_executable(finder_start src/main.cpp)

list(APPEND finder_start_SOURCES
  PRIVATE globals_lib
  PRIVATE display_lib
  Qt5::Widgets
)

target_link_libraries(finder_start ${finder_start_SOURCES} ${LIBS})
