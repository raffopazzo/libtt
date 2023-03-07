#include "libtt/pityp/type.hpp"

#include "libtt/core/var_name_generator.hpp"

#include <algorithm>
#include <iterator>
#include <tuple>

namespace libtt::pityp {

bool type::arr_t::operator==(type::arr_t const& that) const
{
    return std::tie(dom.get(), img.get()) == std::tie(that.dom.get(), that.img.get());
}

bool type::pi_t::operator==(type::pi_t const& that) const
{
    return std::tie(var, body.get()) == std::tie(that.var, that.body.get());
}

type type::var(std::string name)
{
    return type(type::var_t(std::move(name)));
}

type type::arr(type dom, type img)
{
    return type(type::arr_t(std::move(dom), std::move(img)));
}

type type::pi(std::string var_name, type body)
{
    return type::pi(type::var_t(std::move(var_name)), std::move(body));
}

type type::pi(type::var_t var, type body)
{
    return type(type::pi_t(std::move(var), std::move(body)));
}

std::set<type::var_t> free_type_vars(type const& ty)
{
    struct visitor
    {
        std::set<type::var_t> result;

        void operator()(type::var_t const& x)
        {
            result.insert(x);
        }

        void operator()(type::arr_t const& x)
        {
            visit(x.dom.get());
            visit(x.img.get());
        }

        void operator()(type::pi_t const& x)
        {
            auto tmp = free_type_vars(x.body.get());
            tmp.erase(x.var);
            std::ranges::move(tmp, std::inserter(result, result.end()));
        }

        void visit(type const& x)
        {
            std::visit(*this, x.value);
        }
    };

    visitor v;
    v.visit(ty);
    return v.result;
}

std::set<type::var_t> binding_and_free_type_vars(type const& ty)
{
    struct visitor
    {
        std::set<type::var_t> result;

        void operator()(type::var_t const& x)
        {
            result.insert(x);
        }

        void operator()(type::arr_t const& x)
        {
            visit(x.dom.get());
            visit(x.img.get());
        }

        void operator()(type::pi_t const& x)
        {
            result.insert(x.var);
            visit(x.body.get());
        }

        void visit(type const& x)
        {
            std::visit(*this, x.value);
        }
    };

    visitor v;
    v.visit(ty);
    return v.result;
}

type::pi_t rename(type::pi_t const& x, std::set<type::var_t> const& forbidden_names)
{
    auto const& old_vars = binding_and_free_type_vars(x.body.get());
    for (auto const& new_var_name: core::var_name_generator())
    {
        auto const& new_var = type::var_t(new_var_name);
        if (not (old_vars.contains(new_var) or forbidden_names.contains(new_var)))
            return type::pi_t(new_var, replace(x.body.get(), x.var, new_var));
    }
    __builtin_unreachable();
}

type replace(type const& x, type::var_t const& from, type::var_t const& to)
{
    struct visitor
    {
        type::var_t const& from;
        type::var_t const& to;

        type operator()(type::var_t const& x) const
        {
            return type(x == from ? to : x);
        }

        type operator()(type::arr_t const& x) const
        {
            return type::arr(replace(x.dom.get(), from, to), replace(x.img.get(), from, to));
        }

        type operator()(type::pi_t const& x) const
        {
            // Only free occurrences of `from` can be replaced; so if the binding variable is `from`,
            // then all free occurrences of `from` in `body` are now bound and cannot be replaced.
            return x.var == from ? type(x) : type::pi(x.var, replace(x.body.get(), from, to));
        }
    };
    return std::visit(visitor{from, to}, x.value);
}

type substitute(type const& x, type::var_t const& var, type const& y)
{
    struct visitor
    {
        type::var_t const& var;
        type const& y;

        type operator()(type::var_t const& x) const
        {
            return x == var ? y : type(x);
        }

        type operator()(type::arr_t const& x) const
        {
            return type::arr(substitute(x.dom.get(), var, y), substitute(x.img.get(), var, y));
        }

        type operator()(type::pi_t const& x) const
        {
            // only free variables can be substituted
            if (x.var == var)
                return type(x);
            auto const& free_vars_y = free_type_vars(y);
            if (free_vars_y.contains(x.var))
            {
                auto const& renamed = rename(x, free_vars_y);
                return type::pi(renamed.var, substitute(renamed.body.get(), var, y));
            }
            else
                return type::pi(x.var, substitute(x.body.get(), var, y));
        }
    };
    return std::visit(visitor{var, y}, x.value);
}

struct alpha_equivalence_visitor
{
    template <typename T, typename U>
    requires (not std::is_same_v<T, U>)
    bool operator()(T const& x, U const& y) const
    {
        return false;
    }

    bool operator()(type::var_t const& x, type::var_t const& y) const
    {
        return x.name == y.name;
    }

    bool operator()(type::arr_t const& x, type::arr_t const& y) const
    {
        return is_alpha_equivalent(x.dom.get(), y.dom.get()) and is_alpha_equivalent(x.img.get(), y.img.get());
    }

    bool operator()(type::pi_t const& x, type::pi_t const& y) const
    {
        if (x.var == y.var)
            return is_alpha_equivalent(x.body.get(), y.body.get());
        // We now need to check whether `pi u:* . M` is alpha-equivalent to `pi v:* . N`;
        // the naive idea is to compare `M` with `N[v->u]`, but that only works for simple cases;
        // for instance it would fail for `pi u, v : * . u -> v` and `pi v, u : * . v -> u`.
        // But we can use transitivity!
        // We can, in fact, rename both
        //  - `x` to `pi s:* . M[u->s]`
        //  - `y` to `pi s:* . N[v->s]`
        // and then check whether the new body `M[u->s]` is alpha-equivalent to `N[v->s]`.
        // This, of course, requires that `s` be a valid binding variable in both terms once renamed.
        // So when renaming `x` we need to also exclude all free and binding variables in `y`.
        auto const& renamed_x = rename(x, binding_and_free_type_vars(y.body.get()));
        auto const& body_of_renamed_y = replace(y.body.get(), y.var, renamed_x.var);
        return is_alpha_equivalent(renamed_x.body.get(), body_of_renamed_y);
    }
};

bool is_alpha_equivalent(type const& x, type const& y)
{
    return std::visit(alpha_equivalence_visitor{}, x.value, y.value);
}

}
