#include "libtt/untyp/rename.hpp"

#include "libtt/untyp/detail/tmp_var_generator.hpp"
#include "libtt/untyp/replace.hpp"

#include <limits>

namespace libtt::untyp {

term::abs_t rename(term::abs_t const& x, std::set<term::var_t> const& forbidden_names)
{
    auto const& old_vars = binding_and_free_variables(x.body.get());
    for (auto const& new_var: detail::tmp_var_generator())
        if (not (old_vars.contains(new_var) or forbidden_names.contains(new_var)))
            return term::abs_t(new_var, replace(x.body.get(), x.var, new_var));
    __builtin_unreachable();
}

}
