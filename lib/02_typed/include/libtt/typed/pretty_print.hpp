#pragma once

#include "libtt/typed/derivation.hpp"
#include "libtt/typed/pre_typed_term.hpp"
#include "libtt/typed/type.hpp"

#include <ostream>

namespace libtt::typed {

std::ostream& pretty_print(std::ostream&, judgement const&);
std::ostream& pretty_print(std::ostream&, pre_typed_term const&);
std::ostream& pretty_print(std::ostream&, statement const&);
std::ostream& pretty_print(std::ostream&, type const&);

}
