#include "libtt/untyp/substitute.hpp"

#include "libtt/untyp/detail/renaming_context.hpp"
#include "libtt/untyp/rename.hpp"

#include <algorithm>

namespace libtt::untyp {

term substitute(term const& x, term::var_t const& var, term const& y)
{
    struct visitor
    {
	renaming_context& ctx;
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
	    auto const& new_abs = rename(x, ctx);
	    return term::abs(new_abs.var, substitute(new_abs.body.get(), var, y));
	}
    };
    auto ctx = renaming_context([&]
    {
	std::set<term::var_t> forbidden_variables;
	auto inserter = std::inserter(forbidden_variables, forbidden_variables.end());
	std::ranges::move(free_variables(x), inserter);
	std::ranges::move(free_variables(y), inserter);
	std::ranges::move(binding_variables(x), inserter);
	std::ranges::move(binding_variables(y), inserter);
	return forbidden_variables;
    }());
    return std::visit(visitor{ctx, var, y}, x.value);
}

}
