add_executable(pre_typed_term_tests pre_typed_term_tests.cpp)
add_test(NAME pre_typed_term_tests COMMAND pre_typed_term_tests)
target_link_libraries(pre_typed_term_tests PUBLIC libtt::typed)

add_executable(simple_type_tests simple_type_tests.cpp)
add_test(NAME simple_type_tests COMMAND simple_type_tests)
target_link_libraries(simple_type_tests PUBLIC libtt::typed)

add_executable(type_assign_tests type_assign_tests.cpp)
add_test(NAME type_assign_tests COMMAND type_assign_tests)
target_link_libraries(type_assign_tests PUBLIC libtt::typed)

add_executable(term_search_tests term_search_tests.cpp)
add_test(NAME term_search_tests COMMAND term_search_tests)
target_link_libraries(term_search_tests PUBLIC libtt::typed)
