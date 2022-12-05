#include "libtt/untyp/alpha_equivalence.hpp"

#include <algorithm>

namespace libtt::untyp {

static term::var_t do_replace(term::var_t const&, term::var_t const& from, term::var_t const& to);
static term::app_t do_replace(term::app_t const&, term::var_t const& from, term::var_t const& to);
static term::abs_t do_replace(term::abs_t const&, term::var_t const& from, term::var_t const& to);

term::var_t do_replace(term::var_t const& x, term::var_t const& from, term::var_t const& to)
{
    return x == from ? to : x;
}

term::app_t do_replace(term::app_t const& x, term::var_t const& from, term::var_t const& to)
{
    return term::app_t(replace(x.left.get(), from, to), replace(x.right.get(), from, to));
}

term::abs_t do_replace(term::abs_t const& x, term::var_t const& from, term::var_t const& to)
{
    // Only free occurrences of `from` can be replaced; so if the binding variable is `from`,
    // then all free occurrences of `from` in `body` are now bound and cannot be replaced.
    return x.var == from ? x : term::abs_t(x.var, replace(x.body.get(), from, to));
}

term replace(term const& x, term::var_t const& from, term::var_t const& to)
{
    return std::visit([&] (auto const& v) { return term(do_replace(v, from, to)); }, x.value);
}

std::optional<term::abs_t> rename(term::abs_t const& x, term::var_t const& new_var)
{
    auto const& body = x.body.get();
    if (free_variables(body).contains(new_var) or binding_variables(body).contains(new_var))
        return std::nullopt;
    return term::abs_t(new_var, replace(body, x.var, new_var));
}

}
