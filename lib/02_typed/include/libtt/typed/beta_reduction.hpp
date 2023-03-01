#pragma once

#include "libtt/typed/legal_term.hpp"

namespace libtt::typed {

bool is_redex(legal_term const&);
std::size_t num_redexes(legal_term const&);
inline bool is_beta_normal(legal_term const& x) { return num_redexes(x) == 0ul; }

legal_term beta_normalize(legal_term const&);

}
