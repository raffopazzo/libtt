add_executable(untyp_tests untyp_tests.cpp)
add_test(NAME untyp_tests COMMAND untyp_tests)
target_link_libraries(untyp_tests PUBLIC libtt::untyp)
