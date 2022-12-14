#pragma once

#include "libtt/untyp/term.hpp"

#include <set>

namespace libtt::untyp {

term::abs_t rename(term::abs_t const&, std::set<term::var_t> const& forbidden_names = {});

}
