#pragma once

#include "libtt/untyp/term.hpp"

#include <ostream>

namespace libtt::untyp {

std::ostream& pretty_print(std::ostream&, term const&);

}
