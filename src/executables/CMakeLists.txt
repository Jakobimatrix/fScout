if (FUZZER_ENABLED)



else()
  # only enable other executables (like tests), if Fuzzer is not linked. Fuzzer brings its own main!
    
  if(WIN32)
      add_executable(fScout_GUI src/main.cpp icon.rc)
  else()
      add_executable(fScout_GUI src/main.cpp)
  endif()


  list(APPEND fScout_GUI_SOURCES
    PRIVATE globals_lib
    PRIVATE display_lib
    Qt5::Widgets
    ${ENVIRONMENT_SETTINGS}
  )

  # for utlis/system/memory.h
  if(WIN32)
    list(APPEND LIBS psapi)
  endif()

  target_link_libraries(fScout_GUI ${fScout_GUI_SOURCES} ${LIBS})

endif()

