add_executable(derivation_tests derivation_tests.cpp)
add_test(NAME derivation_tests COMMAND derivation_tests)
target_link_libraries(derivation_tests PUBLIC libtt::typed)

add_executable(pre_typed_term_tests pre_typed_term_tests.cpp)
add_test(NAME pre_typed_term_tests COMMAND pre_typed_term_tests)
target_link_libraries(pre_typed_term_tests PUBLIC libtt::typed)

add_executable(simple_type_tests simple_type_tests.cpp)
add_test(NAME simple_type_tests COMMAND simple_type_tests)
target_link_libraries(simple_type_tests PUBLIC libtt::typed)
