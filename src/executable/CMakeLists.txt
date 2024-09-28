add_executable(fScount_GUI src/main.cpp)

list(APPEND fScount_GUI_SOURCES
  PRIVATE globals_lib
  PRIVATE display_lib
  Qt5::Widgets
)

target_link_libraries(fScount_GUI ${fScount_GUI_SOURCES} ${LIBS})
