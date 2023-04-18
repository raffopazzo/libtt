#pragma once

#include "libtt/pityp/pre_typed_term.hpp"
#include "libtt/pityp/type.hpp"

#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace libtt::pityp {

// context

struct context
{
    // In a context, declarations are statements where the
    // subject is either a type variable or a term variable.
    struct type_decl_t
    {
        type::var_t subject;
        bool operator==(type_decl_t const&) const = default;
    };

    struct var_decl_t
    {
        pre_typed_term::var_t subject;
        type ty;

        bool operator==(var_decl_t const&) const = default;
    };

    using decl_t = std::variant<type_decl_t, var_decl_t>;

    bool empty() const { return m_decls.empty(); }
    bool contains(type::var_t const&) const;
    bool contains(pre_typed_term::var_t const&) const;

    std::optional<type_decl_t> operator[](type::var_t const&) const;
    std::optional<var_decl_t> operator[](pre_typed_term::var_t const&) const;

    friend std::vector<decl_t> decls_of(context const&);
    friend std::optional<context> extend(context const&, type::var_t const&);
    friend std::optional<context> extend(context const&, pre_typed_term::var_t const&, type const&);

    bool operator==(context const&) const = default;

private:
    // Both type and term variables must have unique names;
    // so we just map the raw name to either:
    // - a type variable, for a type declaration
    // - or a type, for a variable declariation.
    std::map<std::string, std::variant<type::var_t, type>> m_decls;
};

// judgement

// In a judgement, a statement can be of the form:
// 1. `s : *` where `s` is a type;
// 2. or `M : s`, where `M` is a term and `s` is a type.
// If we define these as "type statement" and "term statement", respectively,
// then it is obvious how to define "type judgement" and "term judgement".
// We are primarly interested in derivations that result from type assignment or term search,
// therefore the conclusion of those derivations will always be "term judgements".
// Finally, a judgement does not have to be derivable, so we can leave the fields public.
// Only a judgement obtained from the conclusion of a derivation is, by definition, derivable.

struct type_stm_t
{
    type subject;
    bool operator==(type_stm_t const&) const = default;
};

struct term_stm_t
{
    pre_typed_term subject;
    type ty;

    // Remove default ctor, to enforce a compile time check that both fields are set.
    term_stm_t(pre_typed_term s, type t) : subject(std::move(s)), ty(std::move(t)) { }

    bool operator==(term_stm_t const&) const = default;
};

template <typename T>
struct judgement
{
    context ctx;
    T stm;

    bool operator==(judgement const&) const = default;
};

using type_judgement_t = judgement<type_stm_t>;
using term_judgement_t = judgement<term_stm_t>;

// derivation

// Here `derivation` is only modeling derivations of terms;
// the derivation of a type is trivially obtained by the formation rule.
// Perhaps we could add some `struct type_derivation` and put the formation rule (and only that) in there;
// if we do, we should probably also add `type_judgement_t conclusion_of(type_derivation const&);` and
// take a `type_derivation` in the ctor of the var rule.
struct derivation
{
    using rec_t = boost::recursive_wrapper<derivation>;

    // NB here we could define `struct form_t` but we omit it because an instance of `derivation` can only
    // be obtained from `type_assign()` or `term_search()`.
    // Therefore, even though the formation rule has a type statement as conclusion,
    // any instance of `derivation` will always have a term statement as conclusion.

    struct var_t
    {
        auto const& ctx() const { return m_ctx; }
        auto const& var() const { return m_var; }
        auto const& ty() const { return m_ty; }

        var_t(var_t const&) = default;
        var_t(var_t&&) = default;
        var_t& operator=(var_t const&) = default;
        var_t& operator=(var_t&&) = default;

    private:
        var_t(context, pre_typed_term::var_t, type);

        context m_ctx;
        pre_typed_term::var_t m_var;
        type m_ty;

        friend struct derivation_rules;
    };

    struct app1_t
    {
        auto const& ctx() const { return m_ctx; }
        auto const& fun() const { return m_fun.get(); } // has a conclusion of arrow type
        auto const& arg() const { return m_arg.get(); } // has a conclusion whose type matches the domain of fun
        auto const& ty() const { return m_ty; }

        app1_t(app1_t const&) = default;
        app1_t(app1_t&&) = default;
        app1_t& operator=(app1_t const&) = default;
        app1_t& operator=(app1_t&&) = default;

    private:
        app1_t(context, derivation, derivation, type);

        context m_ctx;
        rec_t m_fun;
        rec_t m_arg;
        type m_ty;

        friend struct derivation_rules;
    };

    struct app2_t
    {
        auto const& ctx() const { return m_ctx; }
        auto const& fun() const { return m_fun.get(); } // has a conclusion of pi type
        auto const& arg() const { return m_arg; } // the type argument
        auto const& ty() const { return m_ty; } // the resulting type of the application

        app2_t(app2_t const&) = default;
        app2_t(app2_t&&) = default;
        app2_t& operator=(app2_t const&) = default;
        app2_t& operator=(app2_t&&) = default;

    private:
        app2_t(context, derivation, type arg, type ty);

        context m_ctx;
        rec_t m_fun;
        type m_arg;
        type m_ty;

        friend struct derivation_rules;
    };

    struct abs1_t
    {
        auto const& ctx() const { return m_ctx; }
        auto const& var() const { return m_var; }
        auto const& var_type() const { return m_var_type; }
        auto const& body() const { return m_body.get(); }
        auto const& ty() const { return m_ty; }

        abs1_t(abs1_t const&) = default;
        abs1_t(abs1_t&&) = default;
        abs1_t& operator=(abs1_t const&) = default;
        abs1_t& operator=(abs1_t&&) = default;

    private:
        abs1_t(context, pre_typed_term::var_t, type var_type, derivation, type);

        context m_ctx;
        pre_typed_term::var_t m_var;
        type m_var_type;
        rec_t m_body;
        type m_ty;

        friend struct derivation_rules;
    };

    struct abs2_t
    {
        auto const& ctx() const { return m_ctx; }
        auto const& var() const { return m_var; }
        auto const& body() const { return m_body.get(); }
        auto const& ty() const { return m_ty; }

        abs2_t(abs2_t const&) = default;
        abs2_t(abs2_t&&) = default;
        abs2_t& operator=(abs2_t const&) = default;
        abs2_t& operator=(abs2_t&&) = default;

    private:
        abs2_t(context, type::var_t, derivation, type);

        context m_ctx;
        type::var_t m_var;
        rec_t m_body;
        type m_ty;

        friend struct derivation_rules;
    };

    using value_t = std::variant<var_t, app1_t, app2_t, abs1_t, abs2_t>;
    value_t value;

    explicit derivation(var_t);
    explicit derivation(app1_t);
    explicit derivation(app2_t);
    explicit derivation(abs1_t);
    explicit derivation(abs2_t);
};

term_judgement_t conclusion_of(derivation const&);
inline std::optional<term_judgement_t> conclusion_of(std::optional<derivation> const& x)
{
    return x ? std::optional{conclusion_of(*x)} : std::nullopt;
}

// Obtain a derivation, if one exists, whose conclusion is the judgement that the given term has a certain type.
std::optional<derivation> type_assign(context const&, pre_typed_term const&);
std::optional<derivation> term_search(context const&, type const&);
inline bool type_check(context const& ctx, pre_typed_term const& x, type const& t)
{
    auto const conclusion = conclusion_of(type_assign(ctx, x));
    return conclusion and conclusion->stm.ty == t;
}

}
