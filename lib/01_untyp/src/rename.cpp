#include "libtt/untyp/rename.hpp"

#include "libtt/untyp/detail/renaming_context.hpp"
#include "libtt/untyp/replace.hpp"

namespace libtt::untyp {

term::abs_t rename(term::abs_t const& x, renaming_context& ctx)
{
    auto const& new_var = ctx.next_var_name();
    return term::abs_t(new_var, replace(x.body.get(), x.var, new_var));
}

std::optional<term::abs_t> rename(term::abs_t const& x, term::var_t const& new_var)
{
    auto const& body = x.body.get();
    if (free_variables(body).contains(new_var) or binding_variables(body).contains(new_var))
        return std::nullopt;
    return term::abs_t(new_var, replace(body, x.var, new_var));
}

}
