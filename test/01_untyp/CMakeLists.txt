add_executable(untyp_tests untyp_tests.cpp)
add_test(NAME untyp_tests COMMAND untyp_tests)
target_link_libraries(untyp_tests PUBLIC libtt::untyp)

add_executable(untyp_beta_reduction_tests untyp_beta_reduction_tests.cpp)
add_test(NAME untyp_beta_reduction_tests COMMAND untyp_beta_reduction_tests)
target_link_libraries(untyp_beta_reduction_tests PUBLIC libtt::untyp)
