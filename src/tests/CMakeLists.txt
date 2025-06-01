
if (TARGET Catch2::Catch2WithMain)

  add_executable(test_hello_world src/test_hello_world.cpp)

  target_link_libraries(test_hello_world 
    PRIVATE
    Catch2::Catch2WithMain
    library_lib_0.0.1
    ${ENVIRONMENT_SETTINGS}
    )

  

  catch_discover_tests(test_hello_world)


endif()