add_library(libtt_untyp_lib
  include/libtt/untyp/detail/tmp_var_generator.hpp
  include/libtt/untyp/alpha_equivalence.hpp
  include/libtt/untyp/beta_reduction.hpp
  include/libtt/untyp/pretty_print.hpp
  include/libtt/untyp/subterms.hpp
  include/libtt/untyp/term.hpp
  include/libtt/untyp/y_combinator.hpp
  src/detail/tmp_var_generator.cpp
  src/alpha_equivalence.cpp
  src/beta_reduction.cpp
  src/pretty_print.cpp
  src/subterms.cpp
  src/term.cpp
  src/y_combinator.cpp
  )
add_library(libtt::untyp ALIAS libtt_untyp_lib)

target_compile_features(libtt_untyp_lib PUBLIC cxx_std_20)
target_include_directories(libtt_untyp_lib PUBLIC include)
target_link_libraries(libtt_untyp_lib PUBLIC Boost::Boost)
