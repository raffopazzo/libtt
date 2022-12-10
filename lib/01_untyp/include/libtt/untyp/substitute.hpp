#pragma once

#include "libtt/untyp/term.hpp"

namespace libtt::untyp {

term substitute(term const& x, term::var_t const& var, term const& y);

}
