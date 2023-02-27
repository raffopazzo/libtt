#pragma once

#include "libtt/typed/derivation.hpp"

#include <optional>

namespace libtt::typed {

struct legal_term
{
    context ctx;
    pre_typed_term term;
    type ty;

    explicit legal_term(derivation const&);

    bool operator==(legal_term const&) const = default;

private:
    explicit legal_term(judgement&&);
};

std::optional<legal_term> substitute(legal_term const&, pre_typed_term::var_t const&, legal_term const&);

}
