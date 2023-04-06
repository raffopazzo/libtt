#include "libtt/pityp/beta_reduction.hpp"

namespace libtt::pityp {

// is_redex

static bool is_redex(pre_typed_term::var_t const&) { return false; }
static bool is_redex(pre_typed_term::app1_t const& x) { return is_abs1(x.left.get()); }
static bool is_redex(pre_typed_term::app2_t const& x) { return is_abs2(x.left.get()); }
static bool is_redex(pre_typed_term::abs1_t const&) { return false; }
static bool is_redex(pre_typed_term::abs2_t const&) { return false; }
static bool is_redex(pre_typed_term const& x) { return std::visit([] (auto const& x) { return is_redex(x); }, x.value); }
bool is_redex(legal_term const& x) { return is_redex(x.term()); }

// num_redexes

static std::size_t num_redexes(pre_typed_term const&);
static std::size_t num_redexes(pre_typed_term::var_t const& x) { return 0ul; }
static std::size_t num_redexes(pre_typed_term::abs1_t const& x) { return num_redexes(x.body.get()); }
static std::size_t num_redexes(pre_typed_term::abs2_t const& x) { return num_redexes(x.body.get()); }
static std::size_t num_redexes(pre_typed_term::app1_t const& x)
{ 
    return num_redexes(x.left.get()) + num_redexes(x.right.get()) + (is_redex(x) ? 1 : 0);
}
static std::size_t num_redexes(pre_typed_term::app2_t const& x)
{ 
    return num_redexes(x.left.get()) + (is_redex(x) ? 1 : 0);
}
static std::size_t num_redexes(pre_typed_term const& x)
{
    return std::visit([] (auto const& x) { return num_redexes(x); }, x.value);
}

std::size_t num_redexes(legal_term const& x)
{
    return num_redexes(x.term());
}

// beta_normalize
static pre_typed_term one_step_reduction(pre_typed_term const& x)
{
    struct visitor
    {
        pre_typed_term operator()(pre_typed_term::var_t const& x) const
        {
            return pre_typed_term(x);
        }

        pre_typed_term operator()(pre_typed_term::app1_t const& x) const
        {
            if (auto const* const abs = std::get_if<pre_typed_term::abs1_t>(&x.left.get().value))
                return substitute(abs->body.get(), abs->var, x.right.get());
            else
                return pre_typed_term::app(one_step_reduction(x.left.get()), one_step_reduction(x.right.get()));
        }

        pre_typed_term operator()(pre_typed_term::app2_t const& x) const
        {
            if (auto const* const abs = std::get_if<pre_typed_term::abs2_t>(&x.left.get().value))
                return substitute(abs->body.get(), abs->var, x.right);
            else
                return pre_typed_term::app(one_step_reduction(x.left.get()), x.right);
        }

        pre_typed_term operator()(pre_typed_term::abs1_t const& x) const
        {
            return pre_typed_term::abs(x.var, x.var_type, one_step_reduction(x.body.get()));
        }

        pre_typed_term operator()(pre_typed_term::abs2_t const& x) const
        {
            return pre_typed_term::abs(x.var, one_step_reduction(x.body.get()));
        }
    };
    return std::visit(visitor{}, x.value);
}

legal_term beta_normalize(legal_term const& x)
{
    auto res = x.term();
    while (num_redexes(res) > 0ul)
        res = one_step_reduction(res);
    return legal_term(x.ctx(), std::move(res), x.ty());
}

}
