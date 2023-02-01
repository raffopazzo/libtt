add_library(libtt_core_lib STATIC
  include/libtt/core/var_name_generator.hpp
  src/var_name_generator.cpp
  )
add_library(libtt::core ALIAS libtt_core_lib)

target_compile_features(libtt_core_lib PUBLIC cxx_std_20)
target_include_directories(libtt_core_lib PUBLIC include)
