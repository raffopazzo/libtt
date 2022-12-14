#include "libtt/untyp/substitute.hpp"

#include "libtt/untyp/rename.hpp"

namespace libtt::untyp {

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
	    auto const& renamed = rename(x, free_variables(y));
	    return term::abs(renamed.var, substitute(renamed.body.get(), var, y));
	}
    };
    return std::visit(visitor{var, y}, x.value);
}

}
