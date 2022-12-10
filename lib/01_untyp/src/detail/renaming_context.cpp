#include "libtt/untyp/detail/renaming_context.hpp"

namespace libtt::untyp {

renaming_context::renaming_context(std::set<term::var_t> forbidden_variables) :
    forbidden_variables(std::move(forbidden_variables))
{
}

term::var_t renaming_context::next_var_name()
{
    do
    {
        auto const v = term::var_t("tmp_" + std::to_string(next_var_id++));
        if (not forbidden_variables.contains(v))
            return v;
    }
    while (true);
}

}
