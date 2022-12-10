#pragma once

#include "libtt/untyp/term.hpp"

namespace libtt::untyp {

term replace(term const&, term::var_t const& from, term::var_t const& to);

}
