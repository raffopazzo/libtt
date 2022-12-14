#include "libtt/untyp/beta_reduction.hpp"

#include "libtt/untyp/substitute.hpp"

namespace libtt::untyp {

term one_step_beta_reduction(term const& x)
{
    struct visitor
    {
	term operator()(term::var_t const& x)
	{
	    return term(x);
	}

	term operator()(term::app_t const& x)
	{
	    if (auto const* const p_abs = std::get_if<term::abs_t>(&x.left.get().value))
		return substitute(p_abs->body.get(), p_abs->var, x.right.get());
	    else
		return term(x);
	}

	term operator()(term::abs_t const& x)
	{
	    return term(x);
	}
    };
    return std::visit(visitor{}, x.value);
}

}
