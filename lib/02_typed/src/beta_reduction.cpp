#include "libtt/typed/beta_reduction.hpp"

namespace libtt::typed {

// is_redex

static bool is_redex(pre_typed_term::var_t const&) { return false; }
static bool is_redex(pre_typed_term::app_t const& x) { return is_abs(x.left.get()); }
static bool is_redex(pre_typed_term::abs_t const&) { return false; }
static bool is_redex(pre_typed_term const& x) { return std::visit([] (auto const& x) { return is_redex(x); }, x.value); }
bool is_redex(legal_term const& x) { return is_redex(x.term()); }

// num_redexes

static std::size_t num_redexes(pre_typed_term const&);
static std::size_t num_redexes(pre_typed_term::var_t const& x) { return 0ul; }
static std::size_t num_redexes(pre_typed_term::abs_t const& x) { return num_redexes(x.body.get()); }
static std::size_t num_redexes(pre_typed_term::app_t const& x)
{ 
    return num_redexes(x.left.get()) + num_redexes(x.right.get()) + (is_redex(x) ? 1 : 0);
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
legal_term beta_normalize(legal_term const& x)
{
    struct reducing_visitor
    {
        pre_typed_term reduce(pre_typed_term const& x) const
        {
            return std::visit(*this, x.value);
        }

        pre_typed_term operator()(pre_typed_term::var_t const& x) const
        {
            return pre_typed_term(x);
        }

        pre_typed_term operator()(pre_typed_term::app_t const& x) const
        {
            if (is_redex(x))
            {
                auto const& abs = std::get<pre_typed_term::abs_t>(x.left.get().value);
                return reduce(substitute(abs.body.get(), abs.var, x.right.get()));
            }
            else
                return pre_typed_term::app(reduce(x.left.get()), reduce(x.right.get()));
        }

        pre_typed_term operator()(pre_typed_term::abs_t const& x) const
        {
            return pre_typed_term::abs(x.var, x.var_type, reduce(x.body.get()));
        }
    };
    return legal_term(x.ctx(), std::visit(reducing_visitor{}, x.term().value), x.ty());
}

}
