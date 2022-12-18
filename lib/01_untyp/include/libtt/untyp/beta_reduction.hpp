#pragma once

#include "libtt/untyp/term.hpp"

#include <boost/logic/tribool.hpp>

#include <optional>
#include <vector>

namespace libtt::untyp {

bool is_redex(term const&);
std::size_t num_redexes(term const&);
std::vector<term> one_step_beta_reductions(term const&);
std::optional<term> beta_normalize(term const&);

inline bool is_beta_normal(term const& x) { return num_redexes(x) == 0ul; }

// TODO: Would be nice to compare terms with inifnite reduction paths.
// In typed lambda calculus those terms are invalid; so for now tribool will do.
boost::logic::tribool beta_reduces_to(term const&, term const&);
boost::logic::tribool is_beta_equivalent(term const&, term const&);

}
