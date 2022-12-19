add_library(libtt_typed_lib
  include/libtt/typed/type.hpp
  src/type.cpp
  )
add_library(libtt::typed ALIAS libtt_typed_lib)

target_compile_features(libtt_typed_lib PUBLIC cxx_std_20)
target_include_directories(libtt_typed_lib PUBLIC include)
target_link_libraries(libtt_typed_lib PUBLIC Boost::Boost)
