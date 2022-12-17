#pragma once

#include "libtt/untyp/term.hpp"

#include <optional>
#include <vector>

namespace libtt::untyp {

bool is_redux(term const&);
std::size_t num_reduxes(term const&);
std::vector<term> one_step_beta_reduction(term const&);
std::optional<term> beta_reduction(term const&);

inline bool is_beta_normal(term const& x) { return num_reduxes(x) == 0ul; }

}
