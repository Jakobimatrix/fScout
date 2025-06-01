add_library(globals_lib INTERFACE)
target_include_directories(globals_lib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(globals_lib INTERFACE
  timer_lib_1.0.0
  ${ENVIRONMENT_SETTINGS}
  )
