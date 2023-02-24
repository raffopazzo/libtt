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

private:
    explicit legal_term(judgement&&);
};

std::optional<legal_term> substitute(legal_term const&, std::string const&, legal_term const&);

}
