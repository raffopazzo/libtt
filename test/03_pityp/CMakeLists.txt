# add_executable(legal_term_tests legal_term_tests.cpp)
# add_test(NAME legal_term_tests COMMAND legal_term_tests)
# target_link_libraries(legal_term_tests PUBLIC libtt::pityp)
# 
# add_executable(legal_term_beta_reduction_tests legal_term_beta_reduction_tests.cpp)
# add_test(NAME legal_term_beta_reduction_tests COMMAND legal_term_beta_reduction_tests)
# target_link_libraries(legal_term_beta_reduction_tests PUBLIC libtt::pityp)
# 
# add_executable(legal_term_substitution_tests legal_term_substitution_tests.cpp)
# add_test(NAME legal_term_substitution_tests COMMAND legal_term_substitution_tests)
# target_link_libraries(legal_term_substitution_tests PUBLIC libtt::pityp)

add_executable(pi_type_tests pi_type_tests.cpp)
add_test(NAME pi_type_tests COMMAND pi_type_tests)
target_link_libraries(pi_type_tests PUBLIC libtt::pityp)

add_executable(pityp_context_tests pityp_context_tests.cpp)
add_test(NAME pityp_context_tests COMMAND pityp_context_tests)
target_link_libraries(pityp_context_tests PUBLIC libtt::pityp)

add_executable(pityp_derivation_tests pityp_derivation_tests.cpp)
add_test(NAME pityp_derivation_tests COMMAND pityp_derivation_tests)
target_link_libraries(pityp_derivation_tests PUBLIC libtt::pityp)

add_executable(pityp_term_tests pityp_term_tests.cpp)
add_test(NAME pityp_term_tests COMMAND pityp_term_tests)
target_link_libraries(pityp_term_tests PUBLIC libtt::pityp)
# 
# add_executable(term_search_tests term_search_tests.cpp)
# add_test(NAME term_search_tests COMMAND term_search_tests)
# target_link_libraries(term_search_tests PUBLIC libtt::pityp)
