add_executable(fScout_GUI src/main.cpp)

list(APPEND fScout_GUI_SOURCES
  PRIVATE globals_lib
  PRIVATE display_lib
  Qt5::Widgets
)

# for utlis/system/memory.h
if(WIN32)
  list(APPEND LIBS psapi)
endif()

target_link_libraries(fScout_GUI ${fScout_GUI_SOURCES} ${LIBS})
