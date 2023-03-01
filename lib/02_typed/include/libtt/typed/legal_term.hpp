#pragma once

#include "libtt/typed/derivation.hpp"

#include <optional>

namespace libtt::typed {

struct legal_term
{
    auto const& ctx() const { return m_ctx; }
    auto const& term() const { return m_term; }
    auto const& ty() const { return m_ty; }

    explicit legal_term(derivation const&);

    bool operator==(legal_term const&) const = default;

private:
    explicit legal_term(judgement&&);
    legal_term(context, pre_typed_term, type);

    friend legal_term beta_normalize(legal_term const&);

    context m_ctx;
    pre_typed_term m_term;
    type m_ty;
};

std::optional<legal_term> substitute(legal_term const&, pre_typed_term::var_t const&, legal_term const&);

}
