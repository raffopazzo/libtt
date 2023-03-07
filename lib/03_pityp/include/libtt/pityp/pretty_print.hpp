#pragma once

#include "libtt/pityp/derivation.hpp"
#include "libtt/pityp/pre_typed_term.hpp"
#include "libtt/pityp/type.hpp"

#include <ostream>

namespace libtt::pityp {

std::ostream& pretty_print(std::ostream&, context const&);
std::ostream& pretty_print(std::ostream&, judgement<term_stm_t> const&);
std::ostream& pretty_print(std::ostream&, pre_typed_term const&);
std::ostream& pretty_print(std::ostream&, type const&);

}
