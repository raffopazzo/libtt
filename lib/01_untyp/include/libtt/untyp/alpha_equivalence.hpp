#pragma once

#include "libtt/untyp/term.hpp"

#include <optional>

namespace libtt::untyp {

term replace(term const&, term::var_t const& from, term::var_t const& to);
std::optional<term::abs_t> rename(term::abs_t const&, term::var_t const& new_var);
bool is_alpha_equivalent(term const&, term const&);

}
