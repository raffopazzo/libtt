#pragma once

#include "libtt/untyp/term.hpp"

#include <optional>

namespace libtt::untyp {

class renaming_context;

term::abs_t rename(term::abs_t const&, renaming_context&);
std::optional<term::abs_t> rename(term::abs_t const&, term::var_t const& new_var);

}
