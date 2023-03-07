#include "libtt/pityp/derivation.hpp"

#include <algorithm>
#include <vector>

namespace libtt::pityp {

// context

std::vector<context::decl_t> decls_of(context const& ctx)
{
    struct visitor
    {
        std::vector<context::decl_t>& result;
        std::string const& name;

        void operator()(type::var_t const& x) const
        {
            result.push_back(context::type_decl_t(x));
        }
        void operator()(type const& x) const
        {
            result.push_back(context::var_decl_t(pre_typed_term::var_t(name), x));
        }
    };
    std::vector<context::decl_t> result;
    for (auto const& [name, decl]: ctx.m_decls)
        std::visit(visitor{result, name}, decl);
    return result;
}

std::optional<context> extend(context const& ctx, type::var_t const& x)
{
    std::optional<context> res;
    if (not ctx.contains(pre_typed_term::var_t(x.name)))
    {
        // Technically the definition of a context does not allow to declare a type twice,
        // but it really is an idempotent operation, so we do allow it.
        res = ctx;
        res->m_decls.emplace(x.name, x);
    }
    return res;
}

std::optional<context> extend(context const& ctx, pre_typed_term::var_t const& x, type const& ty)
{
    // TODO should check if x not a type declaration
    std::optional<context> res;
    if (not ctx.contains(type::var_t(x.name)) and
        std::ranges::all_of(free_type_vars(ty), [&] (type::var_t const& y) { return ctx.contains(y); }))
    {
        res = ctx;
        res->m_decls.emplace(x.name, ty);
    }
    return res;
}

bool context::contains(type::var_t const& x) const
{
    auto const it = m_decls.find(x.name);
    return it != m_decls.end() and std::holds_alternative<type::var_t>(it->second);
}

bool context::contains(pre_typed_term::var_t const& x) const
{
    auto const it = m_decls.find(x.name);
    return it != m_decls.end() and std::holds_alternative<type>(it->second);
}

std::optional<context::type_decl_t> context::operator[](type::var_t const& x) const
{
    auto const it = m_decls.find(x.name);
    if (it != m_decls.end())
        if (auto const* const p = std::get_if<type::var_t>(&it->second))
            return type_decl_t{*p};
    return std::nullopt;
}

std::optional<context::var_decl_t> context::operator[](pre_typed_term::var_t const& x) const
{
    auto const it = m_decls.find(x.name);
    if (it != m_decls.end())
        if (auto const* const p = std::get_if<type>(&it->second))
            return var_decl_t{x, *p};
    return std::nullopt;
}

// derivation

derivation::var_t::var_t(context ctx, pre_typed_term::var_t var, type ty) :
    m_ctx(std::move(ctx)),
    m_var(std::move(var)),
    m_ty(std::move(ty))
{ }

derivation::app1_t::app1_t( context ctx, derivation fun, derivation arg, type ty) :
    m_ctx(std::move(ctx)),
    m_fun(std::move(fun)),
    m_arg(std::move(arg)),
    m_ty(std::move(ty))
{ }

derivation::app2_t::app2_t( context ctx, derivation fun, type arg, type ty) :
    m_ctx(std::move(ctx)),
    m_fun(std::move(fun)),
    m_arg(std::move(arg)),
    m_ty(std::move(ty))
{ }

derivation::abs1_t::abs1_t(context ctx, pre_typed_term::var_t var, type var_type, derivation body, type ty) :
    m_ctx(std::move(ctx)),
    m_var(std::move(var)),
    m_var_type(std::move(var_type)),
    m_body(std::move(body)),
    m_ty(std::move(ty))
{ }

derivation::abs2_t::abs2_t(context ctx, type::var_t var, derivation body, type ty) :
    m_ctx(std::move(ctx)),
    m_var(std::move(var)),
    m_body(std::move(body)),
    m_ty(std::move(ty))
{ }

derivation::derivation(var_t x) : value(std::move(x)) {}
derivation::derivation(app1_t x) : value(std::move(x)) {}
derivation::derivation(app2_t x) : value(std::move(x)) {}
derivation::derivation(abs1_t x) : value(std::move(x)) {}
derivation::derivation(abs2_t x) : value(std::move(x)) {}

term_judgement_t conclusion_of(derivation const& x)
{
    struct visitor
    {
        term_judgement_t operator()(derivation::var_t const& x) const
        {
            return {x.ctx(), term_stm_t(pre_typed_term(x.var()), x.ty())};
        }

        term_judgement_t operator()(derivation::app1_t const& x) const
        {
            return term_judgement_t(
                x.ctx(),
                term_stm_t(
                    pre_typed_term::app(
                        conclusion_of(x.fun()).stm.subject,
                        conclusion_of(x.arg()).stm.subject),
                    x.ty()));
        }

        term_judgement_t operator()(derivation::app2_t const& x) const
        {
            return term_judgement_t(
                x.ctx(),
                term_stm_t(
                    pre_typed_term::app(
                        conclusion_of(x.fun()).stm.subject,
                        x.arg()),
                    x.ty()));
        }

        term_judgement_t operator()(derivation::abs1_t const& x)
        {
            return term_judgement_t(
                x.ctx(),
                term_stm_t(
                    pre_typed_term::abs(x.var(), x.var_type(), conclusion_of(x.body()).stm.subject),
                    x.ty()));
        }

        term_judgement_t operator()(derivation::abs2_t const& x)
        {
            return term_judgement_t(
                x.ctx(),
                term_stm_t(
                    pre_typed_term::abs(x.var(), conclusion_of(x.body()).stm.subject),
                    x.ty()));
        }
    };
    return std::visit(visitor{}, x.value);
}

std::optional<derivation> type_assign(context const& ctx, pre_typed_term const& x)
{
    struct visitor
    {
        context const& ctx;

        std::optional<derivation> operator()(pre_typed_term::var_t const& x) const
        {
            if (auto decl = ctx[x])
                return derivation(derivation::var_t(ctx, std::move(decl->subject), std::move(decl->ty)));
            else
                return std::nullopt;
        }

        std::optional<derivation> operator()(pre_typed_term::app1_t const& x) const
        {
            auto left_d = type_assign(ctx, x.left.get());
            if (not left_d) return std::nullopt;
            auto const left_conclusion = conclusion_of(*left_d);
            auto p_abs = std::get_if<type::arr_t>(&left_conclusion.stm.ty.value);
            if (not p_abs)  return std::nullopt;
            auto right_d = type_assign(ctx, x.right.get());
            if (not right_d) return std::nullopt;
            auto const right_conclusion = conclusion_of(*right_d);
            if (right_conclusion.stm.ty != p_abs->dom.get()) return std::nullopt;
            return derivation(derivation::app1_t(ctx, std::move(*left_d), std::move(*right_d), p_abs->img.get()));
        }

        std::optional<derivation> operator()(pre_typed_term::app2_t const& x) const
        {
            auto left_d = type_assign(ctx, x.left.get());
            if (not left_d) return std::nullopt;
            auto const left_conclusion = conclusion_of(*left_d);
            auto p_pi = std::get_if<type::pi_t>(&left_conclusion.stm.ty.value);
            if (not p_pi)  return std::nullopt;
            auto const ty = substitute(p_pi->body.get(), p_pi->var, x.right);
            return derivation(derivation::app2_t(ctx, std::move(*left_d), x.right, ty));
        }

        std::optional<derivation> operator()(pre_typed_term::abs1_t const& x) const
        {
            if (auto ext_ctx = extend(ctx, x.var, x.var_type))
                if (auto body = type_assign(*ext_ctx, x.body.get()))
                {
                    // store type of conclusion before moving body
                    auto const ty = type::arr(x.var_type, conclusion_of(*body).stm.ty);
                    return derivation(derivation::abs1_t(ctx, x.var, x.var_type, std::move(*body), ty));
                }
            return std::nullopt;
        }

        std::optional<derivation> operator()(pre_typed_term::abs2_t const& x) const
        {
            if (auto ext_ctx = extend(ctx, x.var))
                if (auto body = type_assign(*ext_ctx, x.body.get()))
                {
                    // store type of conclusion before moving body
                    auto const ty = type::pi(x.var, conclusion_of(*body).stm.ty);
                    return derivation(derivation::abs2_t(ctx, x.var, std::move(*body), ty));
                }
            return std::nullopt;
        }
    };
    return std::visit(visitor{ctx}, x.value);
}

}
