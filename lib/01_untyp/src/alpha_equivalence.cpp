#include "libtt/untyp/alpha_equivalence.hpp"

#include "libtt/untyp/detail/renaming_context.hpp"
#include "libtt/untyp/replace.hpp"


#include <algorithm>
#include <iterator>
#include <ranges>

namespace libtt::untyp {

class alpha_equivalence_visitor
{
    renaming_context ctx;

public:
    alpha_equivalence_visitor(term const& x, term const& y) :
        ctx([&]
        {
            std::set<term::var_t> forbidden_variables;
            auto inserter = std::inserter(forbidden_variables, forbidden_variables.end());
            std::ranges::move(free_variables(x), inserter);
            std::ranges::move(free_variables(y), inserter);
            std::ranges::move(binding_variables(x), inserter);
            std::ranges::move(binding_variables(y), inserter);
            return forbidden_variables;
        }())
    {
    }

    template <typename T, typename U>
    requires (not std::is_same_v<T, U>)
    bool operator()(T const& x, U const& y) const
    {
        return false;
    }

    bool operator()(term::var_t const& x, term::var_t const& y)
    {
        return x.name == y.name;
    }

    bool operator()(term::app_t const& x, term::app_t const& y)
    {
        return is_alpha_equivalent(x.left.get(), y.left.get()) and is_alpha_equivalent(x.right.get(), y.right.get());
    }

    bool operator()(term::abs_t const& x, term::abs_t const& y)
    {
        if (x.var == y.var)
            return is_alpha_equivalent(x.body.get(), y.body.get());
        // we now need to check whether `lambda x . M` is alpha-equivalent to `lambda y . N`;
        // the naive idea would be to rename the latter to `lambda x . N[y->x]` but that only works for simple cases;
        // for instance it would fail for `lambda x y . x y` and `lambda y x . y x`; but we can use transitivity!
        // so we can rename `lambda y . N` to an alpha-equivalent `lambda s . N[y->s]` for some valid `s` and then
        // check whether `M` is alpha-equivalent to `N[y->s]`.
        auto const& tmp = ctx.next_var_name();
        auto const& renamed_x = replace(x.body.get(), x.var, tmp);
        auto const& renamed_y = replace(y.body.get(), y.var, tmp);
        return is_alpha_equivalent(renamed_x, renamed_y);
    }
};

bool is_alpha_equivalent(term const& x, term const& y)
{
    return std::visit(alpha_equivalence_visitor{x, y}, x.value, y.value);
}

}
