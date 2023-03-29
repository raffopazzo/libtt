#include "libtt/pityp/legal_term.hpp"

namespace libtt::pityp {

legal_term::legal_term(derivation const& d) :
    legal_term(conclusion_of(d))
{
}

legal_term::legal_term(term_judgement_t&& j) :
    m_ctx(std::move(j.ctx)),
    m_term(std::move(j.stm.subject)),
    m_ty(std::move(j.stm.ty))
{
}

legal_term::legal_term(context ctx, pre_typed_term term, type ty) :
    m_ctx(std::move(ctx)),
    m_term(std::move(term)),
    m_ty(std::move(ty))
{
}

//std::optional<legal_term> substitute(legal_term const& x, pre_typed_term::var_t const& var, legal_term const& y)
//{
//    // Recomputing a derivation is arguably overkill.
//    // In theory it should be possible to reason directly about the substitution per se.
//    // But this is simpler and guaranteed to work by pure Type Theory.
//    // If it becomes a performance bottleneck we can look into implementing this differently,
//    // which might require `legal_term` to declare `substitute()` as friend.
//    auto type = x.ctx()[var];
//    if (not type)
//        // `var` is not in `x`, so substitution succeeds without changing `x`.
//        return x;
//    if (*type != y.ty())
//        // `y` has different type from `var`, so substitution must fail
//        return std::nullopt;
//
//    auto new_ctx = x.ctx();
//    new_ctx.decls.erase(var);
//    auto const d = type_assign(new_ctx, substitute(x.term(), var, y.term()));
//    if (not d)
//        return std::nullopt;
//    if (x.ty() != std::visit([] (auto const& z) { return z.ty(); }, d->value))
//        return std::nullopt;
//    else
//        return legal_term(*d);
//}

legal_term beta_normalize(legal_term const& x)
{
    return x;
}

}
