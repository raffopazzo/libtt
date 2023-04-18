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
    if (not ctx.contains(x) and not ctx.contains(pre_typed_term::var_t(x.name)))
    {
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

static type type_of(derivation const& d) { return std::visit([](auto const& x) { return x.ty(); }, d.value); }

struct derivation_rules
{
    static derivation var(context ctx, context::var_decl_t decl)
    {
        return derivation(derivation::var_t(std::move(ctx), std::move(decl.subject), std::move(decl.ty)));
    }

    static derivation app1(context ctx, derivation fun, derivation arg)
    {
        auto const fun_type = type_of(fun);
        auto ty = std::get<type::arr_t>(fun_type.value).img.get();
        return derivation(derivation::app1_t(std::move(ctx), std::move(fun), std::move(arg), std::move(ty)));
    }

    static derivation app2(context ctx, derivation pi_term, type arg)
    {
        auto const pi_type = std::get<type::pi_t>(type_of(pi_term).value);
        auto ty = substitute(pi_type.body.get(), pi_type.var, arg);
        return derivation(derivation::app2_t(std::move(ctx), std::move(pi_term), std::move(arg), std::move(ty)));
    }

    static derivation abs1(context ctx, pre_typed_term::var_t var_name, type var_type, derivation body)
    {
        auto&& fun_type = type::arr(var_type, type_of(body));
        return derivation(derivation::abs1_t(
            std::move(ctx),
            std::move(var_name),
            std::move(var_type),
            std::move(body),
            std::move(fun_type)
        ));
    }

    static derivation abs2(context ctx, type::var_t var, derivation body)
    {
        auto&& ty = type::pi(var, type_of(body));
        return derivation(derivation::abs2_t(std::move(ctx), std::move(var), std::move(body), std::move(ty)));
    }
};

std::optional<derivation> type_assign(context const& ctx, pre_typed_term const& x)
{
    struct visitor
    {
        context const& ctx;

        std::optional<derivation> operator()(pre_typed_term::var_t const& x) const
        {
            if (auto decl = ctx[x])
                return derivation_rules::var(ctx, std::move(*decl));
            else
                return std::nullopt;
        }

        std::optional<derivation> operator()(pre_typed_term::app1_t const& x) const
        {
            if (auto left = type_assign(ctx, x.left.get()))
                if (auto const left_type = type_of(*left); is_arr(left_type))
                    if (auto right = type_assign(ctx, x.right.get()))
                        if (type_of(*right) == std::get<type::arr_t>(left_type.value).dom.get())
                            return derivation_rules::app1(ctx, std::move(*left), std::move(*right));
            return std::nullopt;
        }

        std::optional<derivation> operator()(pre_typed_term::app2_t const& x) const
        {
            if (auto left = type_assign(ctx, x.left.get()))
                if (is_pi(type_of(*left)))
                    return derivation_rules::app2(ctx, std::move(*left), x.right);
            return std::nullopt;
        }

        std::optional<derivation> operator()(pre_typed_term::abs1_t const& x) const
        {
            if (auto ext_ctx = extend(ctx, x.var, x.var_type))
                if (auto body = type_assign(*ext_ctx, x.body.get()))
                    return derivation_rules::abs1(ctx, x.var, x.var_type, std::move(*body));
            return std::nullopt;
        }

        std::optional<derivation> operator()(pre_typed_term::abs2_t const& x) const
        {
            if (auto ext_ctx = extend(ctx, x.var))
                if (auto body = type_assign(*ext_ctx, x.body.get()))
                    return derivation_rules::abs2(ctx, x.var, std::move(*body));
            return std::nullopt;
        }
    };
    return std::visit(visitor{ctx}, x.value);
}

static std::vector<context::var_decl_t> var_decls_of(context const& ctx)
{
    std::vector<context::var_decl_t> result;
    for (auto const& decl: decls_of(ctx))
        if (auto const* const p = std::get_if<context::var_decl_t>(&decl))
            result.push_back(*p);
    return result;
}

static std::vector<context::type_decl_t> type_decls_of(context const& ctx)
{
    std::vector<context::type_decl_t> result;
    for (auto const& decl: decls_of(ctx))
        if (auto const* const p = std::get_if<context::type_decl_t>(&decl))
            result.push_back(*p);
    return result;
}

struct term_search_state
{
    std::vector<type> in_progress;
    std::vector<std::pair<type, derivation>> found;
};

static std::optional<derivation> term_search_impl(context const&, type const&, term_search_state&);

static std::vector<derivation> find_all_or_nothing(
    context const& ctx,
    std::vector<type> const& types,
    term_search_state& state)
{
    std::vector<derivation> result;
    for (auto const& t: types)
    {
        if (auto v = term_search_impl(ctx, t, state))
            result.push_back(std::move(*v));
        else
            return {};
    }
    return result;
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

static type final_image_of(type::arr_t const& arr_type)
{
    auto const* p = &arr_type.img.get();
    while (auto const* const q = std::get_if<type::arr_t>(&p->value))
        p = &q->img.get();
    return *p;
}

static std::string generate_fresh_var_name(context const& ctx)
{
    for (auto const& name: core::var_name_generator())
        if (not ctx.contains(pre_typed_term::var_t(name)) and not ctx.contains(type::var_t(name))) 
            return name;
    __builtin_unreachable();
}

static std::optional<derivation> term_search_impl(context const& ctx, type const& target, term_search_state& state)
{
    // We might have already found a term of this type.
    auto const cache_lookup_strategy = [&] () -> std::optional<derivation>
    {
        auto const it = std::ranges::find_if(state.found, [&] (auto const& p) { return p.first == target; });
        if (it != state.found.end())
            return it->second;
        return std::nullopt;
    };

    // The easy case is if there is a term declaration in the given context for the target type.
    auto const var_rule_strategy = [&] () -> std::optional<derivation>
    {
        for (auto const& decl: var_decls_of(ctx))
            if (decl.ty == target)
                return derivation_rules::var(ctx, decl);
        return std::nullopt;
    };

    // If there isn't one, there might still be a function whose image type matches with the target type.
    // If so, it would suffice to find terms in the domain types (or a subset in case of partial application).
    auto const first_order_application_strategy = [&] () -> std::optional<derivation>
    {
        for (auto const& decl: var_decls_of(ctx))
            if (auto const* const p_arr_type = std::get_if<type::arr_t>(&decl.ty.value))
            {
                auto const arg_terms = find_all_or_nothing(ctx, try_split_fun_args(*p_arr_type, target), state);
                if (not arg_terms.empty())
                {
                    auto const first_arg = arg_terms.begin();
                    return std::accumulate(
                        std::next(first_arg), arg_terms.end(),
                        derivation_rules::app1(ctx, derivation_rules::var(ctx, decl), *first_arg),
                        [&ctx] (derivation const& acc, derivation const& arg)
                        {
                            return derivation_rules::app1(ctx, acc, arg);
                        });
                }
            }
        return std::nullopt;
    };

    // Another route is via 2nd order application:
    // we can apply a pi-type term to a type declaration and hope that this returns the target type;
    // it may return instead another pi-type, if so we can try again recursively.
    // This is quite inefficient and limited so needs improving.
    auto const second_order_application_strategy = [&] () -> std::optional<derivation>
    {
        std::vector<type> possible_type_arguments;
        possible_type_arguments.push_back(target);
        for (auto const& decl: type_decls_of(ctx))
            // TODO ignore all type declarations not in the free type variables of the target type;
            // because they will never generate the target type via 2nd order application.
            possible_type_arguments.push_back(type(decl.subject));

        // TODO use "deducing this"
        std::function<std::optional<derivation>(derivation)> try_apply;
        try_apply = [&] (derivation const& pi_term) -> std::optional<derivation>
        {
            auto const pi_type = std::get<type::pi_t>(type_of(pi_term).value);
            for (auto const& type_arg: possible_type_arguments)
            {
                auto app = derivation_rules::app2(ctx, pi_term, type_arg);
                auto const result_type = type_of(app);
                if (result_type == target)
                    return std::move(app);
                else if (is_pi(result_type))
                    // NB no infinite recursion here: we're just consecutively applying the "same" pi-term
                    if (auto d = try_apply(app))
                        return d;
            }
            return std::nullopt;
        };

        for (auto const& decl: var_decls_of(ctx))
        {
            if (is_pi(decl.ty))
            {
                if (auto r = try_apply(derivation_rules::var(ctx, decl)))
                    return r;
            }
            else if (auto const* const p_arr = std::get_if<type::arr_t>(&decl.ty.value))
            {
                // this could still be a, possibly multi-variate, function returning a pi-type; if so, we can search
                // for valid arguments, invoke the function and then perform 2nd order application on its result
                auto const image = final_image_of(*p_arr);
                if (is_pi(image))
                {
                    auto const args = find_all_or_nothing(ctx, try_split_fun_args(*p_arr, image), state);
                    if (not args.empty())
                    {
                        auto const first_arg = args.begin();
                        auto const pi_term =
                            std::accumulate(
                                std::next(first_arg), args.end(),
                                derivation_rules::app1(ctx, derivation_rules::var(ctx, decl), *first_arg),
                                [&ctx] (derivation const& acc, derivation const& arg)
                                {
                                    return derivation_rules::app1(ctx, acc, arg);
                                });
                        if (auto d = try_apply(pi_term))
                            return d;
                    }
                }
            }
        }
        return std::nullopt;
    };

    // If we are looking for a term of a function type or of pi-type,
    // we might be able to assume the argument and deduce the result.
    // NOTE: in either case we are going to perform a new search,
    // but in a new context, so we start afresh with a new state.
    // TODO: by the Thinning Lemma, all the terms we already found
    // in the old context can be found in the new extended context,
    // so we could in principle re-use the terms we already found.
    auto const first_and_second_order_abstraction_strategy = [&] () -> std::optional<derivation>
    {
        struct visitor
        {
            context const& ctx;
            std::optional<derivation> operator()(type::var_t const&) const { return std::nullopt; }
            std::optional<derivation> operator()(type::arr_t const& target) const
            {
                auto new_var = pre_typed_term::var_t(generate_fresh_var_name(ctx));
                if (auto ext_ctx = extend(ctx, new_var, target.dom.get())) // if this fails, the domain type is illegal
                    if (auto d = term_search(std::move(*ext_ctx), target.img.get()))
                        return derivation_rules::abs1(ctx, std::move(new_var), target.dom.get(), std::move(*d));
                return std::nullopt;
            }
            std::optional<derivation> operator()(type::pi_t const& target) const
            {
                // TODO if extension fails, the type variable is already in context or used as term variable;
                // we should try again with a new name
                if (auto ext_ctx = extend(ctx, target.var))
                    if (auto d = term_search(std::move(*ext_ctx), target.body.get()))
                        return derivation_rules::abs2(ctx, target.var, std::move(*d));
                return std::nullopt;
            }
        };
        return std::visit(visitor{ctx}, target.value);
    };

    auto const try_all = [] (auto const&... strategies)
    {
        std::optional<derivation> result;
        auto const try_one = [&] (auto const& strategy) { if (not result) result = strategy(); };
        (try_one(strategies), ...);
        return result;
    };

    if (std::ranges::find(state.in_progress, target) != state.in_progress.end())
        return std::nullopt;

    state.in_progress.push_back(target);
    auto result = try_all(
        cache_lookup_strategy,
        var_rule_strategy,
        first_order_application_strategy,
        second_order_application_strategy,
        first_and_second_order_abstraction_strategy
    );
    state.in_progress.pop_back();
    if (result)
        state.found.push_back(std::make_pair(target, *result));
    return result;
}

std::optional<derivation> term_search(context const& ctx, type const& target)
{
    term_search_state state;
    return term_search_impl(ctx, target, state);
}

}
