add_executable(fScout_GUI src/main.cpp)

list(APPEND fScout_GUI_SOURCES
  PRIVATE globals_lib
  PRIVATE display_lib
  Qt5::Widgets
)

target_link_libraries(fScout_GUI ${fScout_GUI_SOURCES} ${LIBS})
