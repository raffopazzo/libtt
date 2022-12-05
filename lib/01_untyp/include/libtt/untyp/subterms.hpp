#pragma once

#include "libtt/untyp/term.hpp"

#include <vector>

namespace libtt::untyp {

std::vector<term> subterms(term const&);
std::vector<term> proper_subterms(term const&);

}
