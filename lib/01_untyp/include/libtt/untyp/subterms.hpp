#pragma once

#include "libtt/untyp/term.hpp"

#include <vector>

namespace libtt::untyp {

std::vector<term> subterms(term const&);
std::vector<term> proper_subterms(term const&);

// Check if a term is subterm of another, taking alpha-equivalence into account.
bool is_subterm_of(term const&, term const&);

}
