#pragma once

#include "libtt/untyp/term.hpp"

#include <optional>

namespace libtt::untyp {

std::optional<term> one_step_beta_reduction(term const&);

}
