#include "libtt/pityp/pre_typed_term.hpp"

#include "libtt/core/var_name_generator.hpp"

#include <algorithm>
#include <tuple>

namespace libtt::pityp {

bool pre_typed_term::app1_t::operator==(app1_t const& that) const
{
    return std::tie(left.get(), right.get()) == std::tie(that.left.get(), that.right.get());
}

bool pre_typed_term::app2_t::operator==(app2_t const& that) const
{
    return std::tie(left.get(), right) == std::tie(that.left.get(), that.right);
}

bool pre_typed_term::abs1_t::operator==(abs1_t const& that) const
{
    return std::tie(var, var_type, body.get()) == std::tie(that.var, that.var_type, that.body.get());
}

bool pre_typed_term::abs2_t::operator==(abs2_t const& that) const
{
    return std::tie(var, body.get()) == std::tie(that.var, that.body.get());
}

pre_typed_term pre_typed_term::var(std::string name)
{
    return pre_typed_term(var_t(std::move(name)));
}

pre_typed_term pre_typed_term::app(pre_typed_term left, pre_typed_term right)
{
    return pre_typed_term(app1_t(std::move(left), std::move(right)));
}

pre_typed_term pre_typed_term::app(pre_typed_term left, type right)
{
    return pre_typed_term(app2_t(std::move(left), std::move(right)));
}

pre_typed_term pre_typed_term::abs(var_t var_name, type var_type, pre_typed_term body)
{
    return pre_typed_term(abs1_t(std::move(var_name), std::move(var_type), std::move(body)));
}

pre_typed_term pre_typed_term::abs(type::var_t type_var, pre_typed_term body)
{
    return pre_typed_term(abs2_t(std::move(type_var), std::move(body)));
}

std::set<pre_typed_term::var_t> free_variables(pre_typed_term const& x)
{
    struct visitor
    {
        std::set<pre_typed_term::var_t> operator()(pre_typed_term::var_t const& x) const { return {x}; }
        std::set<pre_typed_term::var_t> operator()(pre_typed_term::app1_t const& x) const
        {
            auto left_set = free_variables(x.left.get());
            auto right_set = free_variables(x.right.get());
            left_set.insert(std::make_move_iterator(right_set.begin()), std::make_move_iterator(right_set.end()));
            return left_set;
        }
        std::set<pre_typed_term::var_t> operator()(pre_typed_term::app2_t const& x) const
        {
            return free_variables(x.left.get());
        }
        std::set<pre_typed_term::var_t> operator()(pre_typed_term::abs1_t const& x) const
        {
            auto tmp = free_variables(x.body.get());
            tmp.erase(x.var);
            return tmp;
        }
        std::set<pre_typed_term::var_t> operator()(pre_typed_term::abs2_t const& x) const
        {
            return free_variables(x.body.get());
        }
    };
    return std::visit(visitor{}, x.value);
}

std::set<pre_typed_term::var_t> binding_variables(pre_typed_term const& x)
{
    struct visitor
    {
        std::set<pre_typed_term::var_t> operator()(pre_typed_term::var_t const& x) const { return {}; }
        std::set<pre_typed_term::var_t> operator()(pre_typed_term::app1_t const& x) const
        {
            auto left_set = binding_variables(x.left.get());
            auto right_set = binding_variables(x.right.get());
            left_set.insert(std::make_move_iterator(right_set.begin()), std::make_move_iterator(right_set.end()));
            return left_set;
        }
        std::set<pre_typed_term::var_t> operator()(pre_typed_term::app2_t const& x) const
        {
            return binding_variables(x.left.get());
        }
        std::set<pre_typed_term::var_t> operator()(pre_typed_term::abs1_t const& x) const
        {
            auto tmp = binding_variables(x.body.get());
            tmp.insert(x.var);
            return tmp;
        }
        std::set<pre_typed_term::var_t> operator()(pre_typed_term::abs2_t const& x) const
        {
            return binding_variables(x.body.get());
        }
    };
    return std::visit(visitor{}, x.value);
}

std::set<pre_typed_term::var_t> binding_and_free_variables(pre_typed_term const& x)
{
    std::set<pre_typed_term::var_t> result;
    auto inserter = std::inserter(result, result.end());
    std::ranges::move(free_variables(x), inserter);
    std::ranges::move(binding_variables(x), inserter);
    return result;
}

pre_typed_term::abs1_t rename(pre_typed_term::abs1_t const& x, std::set<pre_typed_term::var_t> const& forbidden_names)
{
    auto const& old_vars = binding_and_free_variables(x.body.get());
    for (auto const& new_var_name: core::var_name_generator())
    {
        auto const new_var = pre_typed_term::var_t(new_var_name);
        if (not (old_vars.contains(new_var) or forbidden_names.contains(new_var)))
            return pre_typed_term::abs1_t(new_var, x.var_type, replace(x.body.get(), x.var, new_var));
    }
    __builtin_unreachable();
}

pre_typed_term replace(pre_typed_term const& x, pre_typed_term::var_t const& from, pre_typed_term::var_t const& to)
{
    struct visitor
    {
        pre_typed_term::var_t const& from;
        pre_typed_term::var_t const& to;

        pre_typed_term operator()(pre_typed_term::var_t const& x) const
        {
            return pre_typed_term(x == from ? to : x);
        }

        pre_typed_term operator()(pre_typed_term::app1_t const& x) const
        {
            return pre_typed_term::app(replace(x.left.get(), from, to), replace(x.right.get(), from, to));
        }

        pre_typed_term operator()(pre_typed_term::app2_t const& x) const
        {
            return pre_typed_term::app(replace(x.left.get(), from, to), x.right);
        }

        pre_typed_term operator()(pre_typed_term::abs1_t const& x) const
        {
            // Only free occurrences of `from` can be replaced; so if the binding variable is `from`,
            // then all free occurrences of `from` in `body` are now bound and cannot be replaced.
            return x.var == from
                ? pre_typed_term(x)
                : pre_typed_term::abs(x.var, x.var_type, replace(x.body.get(), from, to));
        }

        pre_typed_term operator()(pre_typed_term::abs2_t const& x) const
        {
            return pre_typed_term::abs(x.var, replace(x.body.get(), from, to));
        }
    };
    return std::visit(visitor{from, to}, x.value);
}

pre_typed_term substitute(pre_typed_term const& x, pre_typed_term::var_t const& var, pre_typed_term const& y)
{
    struct visitor
    {
        pre_typed_term::var_t const& var;
        pre_typed_term const& y;

        pre_typed_term operator()(pre_typed_term::var_t const& x) const
        {
            return x == var ? y : pre_typed_term(x);
        }

        pre_typed_term operator()(pre_typed_term::app1_t const& x) const
        {
            return pre_typed_term::app(
                substitute(x.left.get(), var, y),
                substitute(x.right.get(), var, y));
        }

        pre_typed_term operator()(pre_typed_term::app2_t const& x) const
        {
            return pre_typed_term::app(
                substitute(x.left.get(), var, y),
                x.right);
        }

        pre_typed_term operator()(pre_typed_term::abs1_t const& x) const
        {
            if (x.var == var)
                // Only free occurrences of `var` can be substituted; so if the binding variable is `var`,
                // then all free occurrences of `var` in `body` are now bound and cannot be substituted.
                return pre_typed_term(x);
            auto const& free_vars_y = free_variables(y);
            if (free_vars_y.contains(x.var))
            {
                auto const& renamed = rename(x, free_vars_y);
                return pre_typed_term::abs(renamed.var, renamed.var_type, substitute(renamed.body.get(), var, y));
            }
            else
                return pre_typed_term::abs(x.var, x.var_type, substitute(x.body.get(), var, y));
        }

        pre_typed_term operator()(pre_typed_term::abs2_t const& x) const
        {
            return pre_typed_term::abs(x.var, substitute(x.body.get(), var, y));
        }
    };
    return std::visit(visitor{var, y}, x.value);
}

pre_typed_term substitute(pre_typed_term const& x, type::var_t const& var, type const& ty)
{
    struct visitor
    {
        type::var_t const& var;
        type const& ty;

        pre_typed_term operator()(pre_typed_term::var_t const& x) const { return pre_typed_term(x); }

        pre_typed_term operator()(pre_typed_term::app1_t const& x) const
        {
            return pre_typed_term::app(
                substitute(x.left.get(), var, ty),
                substitute(x.right.get(), var, ty));
        }

        pre_typed_term operator()(pre_typed_term::app2_t const& x) const
        {
            return pre_typed_term::app(
                substitute(x.left.get(), var, ty),
                substitute(x.right, var, ty));
        }

        pre_typed_term operator()(pre_typed_term::abs1_t const& x) const
        {
            return pre_typed_term::abs(
                x.var,
                substitute(x.var_type, var, ty),
                substitute(x.body.get(), var, ty));
        }

        pre_typed_term operator()(pre_typed_term::abs2_t const& x) const
        {
            if (x.var == var)
                // Only free occurrences of `var` can be substituted; so if the binding variable is `var`,
                // then all free occurrences of `var` in `body` are now bound and cannot be substituted.
                return pre_typed_term(x);
            else
                return pre_typed_term::abs(x.var, substitute(x.body.get(), var, ty));
        }
    };
    return std::visit(visitor{var, ty}, x.value);
}

}
