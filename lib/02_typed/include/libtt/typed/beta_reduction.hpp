#pragma once

#include "libtt/typed/legal_term.hpp"

#include <optional>
#include <vector>

namespace libtt::typed {

bool is_redex(legal_term const&);
std::size_t num_redexes(legal_term const&);
std::vector<legal_term> one_step_beta_reductions(legal_term const&);
legal_term beta_normalize(legal_term const&);

inline bool is_beta_normal(legal_term const& x) { return num_redexes(x) == 0ul; }

bool beta_reduces_to(legal_term const&, legal_term const&);
bool is_beta_equivalent(legal_term const&, legal_term const&);

}
