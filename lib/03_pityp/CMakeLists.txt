add_library(libtt_pityp_lib
# include/libtt/pityp/alpha_equivalence.hpp
  include/libtt/pityp/beta_reduction.hpp
  include/libtt/pityp/derivation.hpp
  include/libtt/pityp/legal_term.hpp
  include/libtt/pityp/pretty_print.hpp
  include/libtt/pityp/pre_typed_term.hpp
  include/libtt/pityp/type.hpp
# src/alpha_equivalence.cpp
  src/beta_reduction.cpp
  src/derivation.cpp
  src/legal_term.cpp
  src/pretty_print.cpp
  src/pre_typed_term.cpp
  src/type.cpp
  )
add_library(libtt::pityp ALIAS libtt_pityp_lib)

target_compile_features(libtt_pityp_lib PUBLIC cxx_std_20)
target_include_directories(libtt_pityp_lib PUBLIC include)
target_link_libraries(libtt_pityp_lib
  PUBLIC
    Boost::Boost
  PRIVATE
    libtt::core
)
