#pragma once

#include "libtt/pityp/derivation.hpp"
#include "libtt/pityp/legal_term.hpp"
#include "libtt/pityp/pre_typed_term.hpp"
#include "libtt/pityp/type.hpp"

#include <ostream>

namespace libtt::pityp {

std::ostream& pretty_print(std::ostream&, context const&);
std::ostream& pretty_print(std::ostream&, context::type_decl_t const&);
std::ostream& pretty_print(std::ostream&, context::var_decl_t const&);
std::ostream& pretty_print(std::ostream&, context::decl_t const&);
std::ostream& pretty_print(std::ostream&, judgement<term_stm_t> const&);
std::ostream& pretty_print(std::ostream&, legal_term const&);
std::ostream& pretty_print(std::ostream&, pre_typed_term const&);
std::ostream& pretty_print(std::ostream&, type const&);
std::ostream& pretty_print(std::ostream&, type::var_t const&);
std::ostream& pretty_print(std::ostream&, type::arr_t const&);
std::ostream& pretty_print(std::ostream&, type::pi_t const&);

}
