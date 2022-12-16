#include "libtt/untyp/beta_reduction.hpp"

#include "libtt/untyp/substitute.hpp"

namespace libtt::untyp {

std::optional<term> one_step_beta_reduction(term const& x)
{
    struct visitor
    {
        std::optional<term> operator()(term::var_t const& x)
        {
            return std::nullopt;
        }

        std::optional<term> operator()(term::app_t const& x)
        {
            if (auto const* const p_abs = std::get_if<term::abs_t>(&x.left.get().value))
                return substitute(p_abs->body.get(), p_abs->var, x.right.get());
            if (auto contractum = one_step_beta_reduction(x.left.get()))
                return term::app(std::move(*contractum), x.right.get());
            if (auto contractum = one_step_beta_reduction(x.right.get()))
                return term::app(x.left.get(), std::move(*contractum));
            return std::nullopt;
        }

        std::optional<term> operator()(term::abs_t const& x)
        {
            if (auto contractum = one_step_beta_reduction(x.body.get()))
                return term::abs(x.var, std::move(*contractum));
            else
                return std::nullopt;
        }
    };
    return std::visit(visitor{}, x.value);
}

}
