#include "libtt/typed/legal_term.hpp"

namespace libtt::typed {

legal_term::legal_term(derivation const& d) :
    legal_term(conclusion_of(d))
{
}

legal_term::legal_term(judgement&& j) :
    ctx(std::move(j.ctx)),
    term(std::move(j.stm.subject)),
    ty(std::move(j.stm.ty))
{
}

std::optional<legal_term> substitute(legal_term const&, std::string const&, legal_term const&)
{
}

}
