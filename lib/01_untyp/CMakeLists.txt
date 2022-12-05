add_library(libtt_untyp_lib
  include/libtt/untyp/alpha_equivalence.hpp
  include/libtt/untyp/pretty_print.hpp
  include/libtt/untyp/subterms.hpp
  include/libtt/untyp/term.hpp
  include/libtt/untyp.hpp
  src/alpha_equivalence.cpp
  src/pretty_print.cpp
  src/subterms.cpp
  src/term.cpp
  src/untyp.cpp
  )
add_library(libtt::untyp ALIAS libtt_untyp_lib)

target_compile_features(libtt_untyp_lib PUBLIC cxx_std_20)
target_include_directories(libtt_untyp_lib PUBLIC include)
target_link_libraries(libtt_untyp_lib PUBLIC Boost::Boost)
