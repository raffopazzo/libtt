add_library(libtt_typed_lib
  include/libtt/typed/alpha_equivalence.hpp
  include/libtt/typed/beta_reduction.hpp
  include/libtt/typed/derivation.hpp
  include/libtt/typed/legal_term.hpp
  include/libtt/typed/pretty_print.hpp
  include/libtt/typed/pre_typed_term.hpp
  include/libtt/typed/type.hpp
  src/alpha_equivalence.cpp
  src/beta_reduction.cpp
  src/derivation.cpp
  src/legal_term.cpp
  src/pretty_print.cpp
  src/pre_typed_term.cpp
  src/type.cpp
  )
add_library(libtt::typed ALIAS libtt_typed_lib)

target_compile_features(libtt_typed_lib PUBLIC cxx_std_20)
target_include_directories(libtt_typed_lib PUBLIC include)
target_link_libraries(libtt_typed_lib
  PUBLIC
    Boost::Boost
  PRIVATE
    libtt::core
)
