add_executable(simple_type_tests simple_type_tests.cpp)
add_test(NAME simple_type_tests COMMAND simple_type_tests)
target_link_libraries(simple_type_tests PUBLIC libtt::typed)
