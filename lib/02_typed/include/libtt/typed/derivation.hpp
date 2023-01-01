#pragma once

#include "libtt/typed/pre_typed_term.hpp"

#include <boost/variant/recursive_wrapper.hpp>

#include <map>
#include <optional>
#include <variant>
#include <vector>

namespace libtt::typed {

struct statement
{
    pre_typed_term subject;
    type ty;

    bool operator==(statement const&) const = default;
};

struct context
{
    std::map<pre_typed_term::var_t, type> decls;

    std::optional<type> operator[](pre_typed_term::var_t const&) const;

    bool operator==(context const&) const = default;
};

// A judgement does not have to be derivable, so we can leave the fields public.
// Only a judgement obtained from the conclusion of a derivation is, by definition, derivable.
struct judgement
{
    context ctx;
    statement stm;

    bool operator==(judgement const&) const = default;
};

class derivation
{
    // std::vector<judgement> m_premisses;
    // judgement m_conclusion;
    derivation() = delete;
    friend std::optional<derivation> type_assign(context const&, pre_typed_term const&);
    // friend std::optional<derivation> term_search(context const&, type const&);

public:
    using rec_t = boost::recursive_wrapper<derivation>;

    struct var_t
    {
        context ctx;
        pre_typed_term::var_t var;
        type ty;
    };

    struct app_t
    {
        context ctx;
        rec_t left;
        rec_t right;
        type ty;
    };

    struct abs_t
    {
        context ctx;
        pre_typed_term::var_t var;
        type var_type;
        rec_t body;
        type ty;
    };

    using value_t = std::variant<var_t, app_t, abs_t>;
    value_t value;

    // std::vector<judgement> premisses() const;
    judgement conclusion() const;

private:
    explicit derivation(var_t);
    explicit derivation(app_t);
    explicit derivation(abs_t);
};

std::optional<derivation> type_assign(context const&, pre_typed_term const&);
std::optional<derivation> term_search(context const&, type const&);

inline std::optional<judgement> conclusion_of(std::optional<derivation> const& x)
{
    return x ? std::optional{x->conclusion()} : std::nullopt;
}

inline bool type_check(context const& ctx, pre_typed_term const& x, type const& t)
{
    auto const d = type_assign(ctx, x);
    return d and d->conclusion().stm.ty == t;
}

}
