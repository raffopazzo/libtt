#pragma once

#include "libtt/untyp/term.hpp"

#include <optional>
#include <vector>

namespace libtt::untyp {

bool is_redex(term const&);
std::size_t num_redexes(term const&);
std::vector<term> one_step_beta_reductions(term const&);
std::optional<term> beta_reduction(term const&);

inline bool is_beta_normal(term const& x) { return num_redexes(x) == 0ul; }

}
