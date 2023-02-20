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

struct derivation
{
    using rec_t = boost::recursive_wrapper<derivation>;

    // The member variable `ty` in `var_t`, `app_t` and `abs_t` is technically redundant because its value can always
    // be derived ether by a lookup in the context (for the var-case) or by inspecting the conclusions of the premisses.

    struct var_t
    {
        context ctx;
        pre_typed_term::var_t var;
        type ty;

    private:
        var_t(context, pre_typed_term::var_t, type);

        friend std::optional<derivation> type_assign(context const&, pre_typed_term const&);
        friend std::optional<derivation> term_search(context const&, type const&);
    };

    struct app_t
    {
        context ctx;
        rec_t fun; // the conclusion of this derivation contains a statement of arrow type
        rec_t arg; // the conclusion of this derivation contains a statement whose type matches the domain of the arrow
        type ty;

    private:
        app_t(context, derivation, derivation, type ty);

        friend std::optional<derivation> type_assign(context const&, pre_typed_term const&);
        friend std::optional<derivation> term_search(context const&, type const&);
    };

    struct abs_t
    {
        context ctx;
        pre_typed_term::var_t var;
        type var_type;
        rec_t body;
        type ty;

    private:
        abs_t(context, pre_typed_term::var_t, type, derivation, type);

        friend std::optional<derivation> type_assign(context const&, pre_typed_term const&);
        friend std::optional<derivation> term_search(context const&, type const&);
    };

    using value_t = std::variant<var_t, app_t, abs_t>;
    value_t value;

    explicit derivation(var_t);
    explicit derivation(app_t);
    explicit derivation(abs_t);
};

judgement conclusion_of(derivation const&);
inline std::optional<judgement> conclusion_of(std::optional<derivation> const& x)
{
    return x ? std::optional{conclusion_of(*x)} : std::nullopt;
}

std::optional<derivation> type_assign(context const&, pre_typed_term const&);
std::optional<derivation> term_search(context const&, type const&);
inline bool type_check(context const& ctx, pre_typed_term const& x, type const& t)
{
    auto const conclusion = conclusion_of(type_assign(ctx, x));
    return conclusion and conclusion->stm.ty == t;
}

}
