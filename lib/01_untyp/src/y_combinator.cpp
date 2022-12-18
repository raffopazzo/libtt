#include "libtt/untyp/y_combinator.hpp"

#include "libtt/untyp/substitute.hpp"

namespace libtt::untyp {

term y_combinator(term const& x)
{
    auto const var_x = term::var("x");
    auto const var_y = term::var("y");
    auto const xx = term::app(var_x, var_x);
    auto const half = term::abs("x", term::app(var_y, xx));
    return substitute(term::app(half, half), term::var_t("y"), x);
}

}
