#pragma once

#include "libtt/untyp/term.hpp"

#include <set>

namespace libtt::untyp {

class renaming_context
{
    std::set<term::var_t> forbidden_variables;
    std::size_t next_var_id = 0ul;

public:
    renaming_context(std::set<term::var_t> forbidden_variables);

    term::var_t next_var_name();
};

}
