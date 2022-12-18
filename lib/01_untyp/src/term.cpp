#include "libtt/untyp/term.hpp"

#include "libtt/untyp/detail/tmp_var_generator.hpp"

#include <algorithm>

namespace libtt::untyp {

bool term::app_t::operator==(term::app_t const& that) const
{
    return left.get() == that.left.get() && right.get() == that.right.get();
}

bool term::abs_t::operator==(term::abs_t const& that) const
{
    return var == that.var and body.get() == that.body.get();
}

term term::var(std::string name)
{
    return term(var_t(std::move(name)));
}

term term::app(term x, term y)
{
    return term(app_t(std::move(x), std::move(y)));
}

term term::abs(std::string var_name, term body)
{
    return term(abs_t(var_t(std::move(var_name)), std::move(body)));
}

term term::abs(var_t name, term body)
{
    return term(abs_t(std::move(name), std::move(body)));
}

bool is_var(term const& x) { return std::holds_alternative<term::var_t>(x.value); }
bool is_app(term const& x) { return std::holds_alternative<term::app_t>(x.value); }
bool is_abs(term const& x) { return std::holds_alternative<term::abs_t>(x.value); }

bool is_closed(term const& x) { return free_variables(x).empty(); }

std::set<term::var_t> free_variables(term const& x)
{
    struct visitor
    {
        std::set<term::var_t> operator()(term::var_t const& x) const { return {x}; }
        std::set<term::var_t> operator()(term::app_t const& x) const
        {
            auto left_set = free_variables(x.left.get());
            auto right_set = free_variables(x.right.get());
            left_set.insert(std::make_move_iterator(right_set.begin()), std::make_move_iterator(right_set.end()));
            return left_set;
        }
        std::set<term::var_t> operator()(term::abs_t const& x) const
        {
            auto tmp = free_variables(x.body.get());
            tmp.erase(x.var);
            return tmp;
        }
    };
    return std::visit(visitor{}, x.value);
}

std::set<term::var_t> binding_variables(term const& x)
{
    struct visitor
    {
        std::set<term::var_t> operator()(term::var_t const& x) const { return {}; }
        std::set<term::var_t> operator()(term::app_t const& x) const
        {
            auto left_set = binding_variables(x.left.get());
            auto right_set = binding_variables(x.right.get());
            left_set.insert(std::make_move_iterator(right_set.begin()), std::make_move_iterator(right_set.end()));
            return left_set;
        }
        std::set<term::var_t> operator()(term::abs_t const& x)
        {
            auto tmp = binding_variables(x.body.get());
            tmp.insert(x.var);
            return tmp;
        }
    };
    return std::visit(visitor{}, x.value);
}

std::set<term::var_t> binding_and_free_variables(term const& x)
{
    std::set<term::var_t> result;
    auto inserter = std::inserter(result, result.end());
    std::ranges::move(free_variables(x), inserter);
    std::ranges::move(binding_variables(x), inserter);
    return result;
}
term::abs_t rename(term::abs_t const& x, std::set<term::var_t> const& forbidden_names)
{
    auto const& old_vars = binding_and_free_variables(x.body.get());
    for (auto const& new_var: detail::tmp_var_generator())
        if (not (old_vars.contains(new_var) or forbidden_names.contains(new_var)))
            return term::abs_t(new_var, replace(x.body.get(), x.var, new_var));
    __builtin_unreachable();
}

term replace(term const& x, term::var_t const& from, term::var_t const& to)
{
    struct visitor
    {
        term::var_t const& from;
        term::var_t const& to;

        term operator()(term::var_t const& x) const
        {
            return term(x == from ? to : x);
        }

        term operator()(term::app_t const& x) const
        {
            return term::app(replace(x.left.get(), from, to), replace(x.right.get(), from, to));
        }

        term operator()(term::abs_t const& x) const
        {
            // Only free occurrences of `from` can be replaced; so if the binding variable is `from`,
            // then all free occurrences of `from` in `body` are now bound and cannot be replaced.
            return x.var == from ? term(x) : term::abs(x.var, replace(x.body.get(), from, to));
        }
    };
    return std::visit(visitor{from, to}, x.value);
}

term substitute(term const& x, term::var_t const& var, term const& y)
{
    struct visitor
    {
	term::var_t const& var;
	term const& y;

	term operator()(term::var_t const& x)
	{
	    return x == var ? y : term(x);
	}

	term operator()(term::app_t const& x)
	{
	    return term::app(
		substitute(x.left.get(), var, y),
		substitute(x.right.get(), var, y));
	}

	term operator()(term::abs_t const& x)
	{
	    if (x.var == var)
		// Only free occurrences of `var` can be substituted; so if the binding variable is `var`,
		// then all free occurrences of `var` in `body` are now bound and cannot be substituted.
		return term::abs(x.var, x.body.get());
	    auto const& free_vars_y = free_variables(y);
	    if (free_vars_y.contains(x.var))
	    {
		auto const& renamed = rename(x, free_vars_y);
		return term::abs(renamed.var, substitute(renamed.body.get(), var, y));
	    }
	    else
		return term::abs(x.var, substitute(x.body.get(), var, y));
	}
    };
    return std::visit(visitor{var, y}, x.value);
}

}
