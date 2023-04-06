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

}
