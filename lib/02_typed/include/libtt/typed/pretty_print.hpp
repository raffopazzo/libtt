#pragma once

#include "libtt/typed/type.hpp"

#include <ostream>

namespace libtt::typed {

std::ostream& pretty_print(std::ostream&, type const&);

}
