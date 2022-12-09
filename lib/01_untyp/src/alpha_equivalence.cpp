#include "libtt/untyp/alpha_equivalence.hpp"

#include <algorithm>
#include <iterator>
#include <ranges>

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

class alpha_equivalence_visitor
{
    std::set<term::var_t> forbidden_variables;
    std::size_t next_var_id = 0ul;

public:
    alpha_equivalence_visitor(term const& x, term const& y)
    {
        auto inserter = std::inserter(forbidden_variables, forbidden_variables.end());
        std::ranges::move(free_variables(x), inserter);
        std::ranges::move(free_variables(y), inserter);
        std::ranges::move(binding_variables(x), inserter);
        std::ranges::move(binding_variables(y), inserter);
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
        auto const& tmp = next_var_name();
        auto const& renamed_x = replace(x.body.get(), x.var, tmp);
        auto const& renamed_y = replace(y.body.get(), y.var, tmp);
        return is_alpha_equivalent(renamed_x, renamed_y);
    }

private:
    term::var_t next_var_name()
    {
        do
        {
            auto const v = term::var_t("tmp_" + std::to_string(next_var_id++));
            if (not forbidden_variables.contains(v))
                return v;
        }
        while (true);
    }
};

bool is_alpha_equivalent(term const& x, term const& y)
{
    return std::visit(alpha_equivalence_visitor{x, y}, x.value, y.value);
}

}
