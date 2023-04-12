#include "libtt/pityp/derivation.hpp"
#include "libtt/core/var_name_generator.hpp"

#include <algorithm>
#include <numeric>
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
    std::optional<context> res;
    if (not ctx.contains(x) and not ctx.contains(type::var_t(x.name)) and
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

static std::vector<type> try_split_fun_args(type::arr_t const& arr_type, type const& image_type)
{
    auto const* p_arr_type = &arr_type;
    std::vector<type> result;
    result.push_back(p_arr_type->dom.get());
    while (p_arr_type->img.get() != image_type)
    {
        p_arr_type = std::get_if<type::arr_t>(&p_arr_type->img.get().value);
        if (p_arr_type == nullptr)
            return {};
        result.push_back(p_arr_type->dom.get());
    }
    return result;
}

static std::vector<derivation> find_all_or_nothing(context const& ctx, std::vector<type> const& types)
{
    std::vector<derivation> result;
    for (auto const& t: types)
        if (auto v = term_search(ctx, t))
            result.push_back(std::move(*v));
        else
            return {};
    return result;
}

static std::string generate_fresh_var_name(context const& ctx)
{
    for (auto const& name: core::var_name_generator())
        if (not ctx.contains(pre_typed_term::var_t(name)) and not ctx.contains(type::var_t(name))) 
            return name;
    __builtin_unreachable();
}

std::optional<derivation> term_search(context const& ctx, type const& target)
{
    // TODO could check if type is legal in this context, otherwise bail out right away

    std::vector<context::var_decl_t> var_decls;
    for (auto const& d: decls_of(ctx))
        if (auto const* const p_var = std::get_if<context::var_decl_t>(&d))
            var_decls.push_back(*p_var);

    auto const var_rule = [&] (context::var_decl_t const& decl)
    {
        return derivation(derivation::var_t(ctx, decl.subject, decl.ty));
    };

    // The easy case is if there is a term declaration in the given context for the target type.
    for (auto const& decl: var_decls)
        if (decl.ty == target)
            return var_rule(decl);

    // There isn't one. But there might be a function whose image type matches with the target type.
    // If so, it suffices to find terms in the domain types (or a subset in case of partial application).
    for (auto const& decl: var_decls)
        if (auto const* const p_arr_type = std::get_if<type::arr_t>(&decl.ty.value))
        {
            auto const arg_terms = find_all_or_nothing(ctx, try_split_fun_args(*p_arr_type, target));
            if (not arg_terms.empty())
            {
                auto const first_arg = arg_terms.begin();
                return std::accumulate(
                    std::next(first_arg), arg_terms.end(),
                    derivation(derivation::app1_t(ctx, var_rule(decl), *first_arg, p_arr_type->img.get())),
                    [&ctx] (derivation const& acc, derivation const& arg)
                    {
                        // Because we are accumulating over multi-variate function, acc must also be of arrow type,
                        // and the result of this application will be of the type of the image.
                        // Also, taking copy because otherwise, std::get would introduce a dangling reference
                        // to the temporary judgement from conclusion_of(acc).
                        auto const arr = std::get<type::arr_t>(conclusion_of(acc).stm.ty.value);
                        return derivation(derivation::app1_t(ctx, acc, arg, arr.img.get()));
                    });
            }
        }

    // Another route is via 2nd order application: we can check if substitution inside pi-types yields the target type.
    // TODO

    // So far we could not produce a term of the target type, not even by function application.
    // If we are trying to produce a simple type, then there is nothing else we can do.
    // But if we are trying to search for a term of some function type, we might be able to assume a term
    // of the domain type and with that produce a term of the image type.
    struct visitor
    {
        context const& ctx;

        std::optional<derivation> operator()(type::var_t const& target) const { return std::nullopt; }

        std::optional<derivation> operator()(type::arr_t const& target) const
        {
            auto new_var = pre_typed_term::var_t(generate_fresh_var_name(ctx));
            if (auto ext_ctx = extend(ctx, new_var, target.dom.get())) // if this fails, the domain type is an illegal
                if (auto d = term_search(std::move(*ext_ctx), target.img.get()))
                    return derivation(
                        derivation::abs1_t(
                            ctx,
                            std::move(new_var),
                            target.dom.get(),
                            std::move(*d),
                            type(target)));
            return std::nullopt;
        }

        // TODO can we do anything for pi-types? if not add to the comment above
        std::optional<derivation> operator()(type::pi_t const& target) const { return std::nullopt; }
    };
    return std::visit(visitor{ctx}, target.value);
}

}
