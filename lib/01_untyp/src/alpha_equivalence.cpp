#include "libtt/untyp/alpha_equivalence.hpp"

#include "libtt/untyp/rename.hpp"
#include "libtt/untyp/replace.hpp"

#include <type_traits>

namespace libtt::untyp {

struct alpha_equivalence_visitor
{
    template <typename T, typename U>
    requires (not std::is_same_v<T, U>)
    bool operator()(T const& x, U const& y) const
    {
        return false;
    }

    bool operator()(term::var_t const& x, term::var_t const& y) const
    {
        return x.name == y.name;
    }

    bool operator()(term::app_t const& x, term::app_t const& y) const
    {
        return is_alpha_equivalent(x.left.get(), y.left.get()) and is_alpha_equivalent(x.right.get(), y.right.get());
    }

    bool operator()(term::abs_t const& x, term::abs_t const& y) const
    {
        if (x.var == y.var)
            return is_alpha_equivalent(x.body.get(), y.body.get());
        // We now need to check whether `lambda u . M` is alpha-equivalent to `lambda v . N`;
        // the naive idea is to compare `M` with `N[v->u]`, but that only works for simple cases;
        // for instance it would fail for `lambda u v . u v` and `lambda v u . v u`.
        // But we can use transitivity!
        // We can, in fact, rename both
        //  - `x` to `lambda s . M[u->s]`
        //  - `y` to `lambda s . N[v->s]`
        // and then check whether the new body `M[u->s]` is alpha-equivalent to `N[v->s]`.
        // This, of course, requires that `s` be a valid binding variable in both terms once renamed.
        // So when renaming `x` we need to also exclude all free and binding variables in `y`.
        auto const& renamed_x = rename(x, binding_and_free_variables(y.body.get()));
        auto const& body_of_renamed_y = replace(y.body.get(), y.var, renamed_x.var);
        return is_alpha_equivalent(renamed_x.body.get(), body_of_renamed_y);
    }
};

bool is_alpha_equivalent(term const& x, term const& y)
{
    return std::visit(alpha_equivalence_visitor{}, x.value, y.value);
}

}
