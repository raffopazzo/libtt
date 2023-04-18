#include "libtt/typed/derivation.hpp"
#include "libtt/core/var_name_generator.hpp"

#include <algorithm>
#include <numeric>

namespace libtt::typed {

std::optional<type> context::operator[](pre_typed_term::var_t const& v) const
{
    auto const it = decls.find(v);
    return it == decls.end() ? std::nullopt : std::optional{it->second};
}

derivation::var_t::var_t(context ctx, pre_typed_term::var_t var, type ty) :
    m_ctx(std::move(ctx)),
    m_var(std::move(var)),
    m_ty(std::move(ty))
{ }

derivation::app_t::app_t( context ctx, derivation fun, derivation arg, type ty) :
    m_ctx(std::move(ctx)),
    m_fun(std::move(fun)),
    m_arg(std::move(arg)),
    m_ty(std::move(ty))
{ }

derivation::abs_t::abs_t(context ctx, pre_typed_term::var_t var, type var_type, derivation body, type ty) :
    m_ctx(std::move(ctx)),
    m_var(std::move(var)),
    m_var_type(std::move(var_type)),
    m_body(std::move(body)),
    m_ty(std::move(ty))
{ }

derivation::derivation(var_t x) : value(std::move(x)) {}
derivation::derivation(app_t x) : value(std::move(x)) {}
derivation::derivation(abs_t x) : value(std::move(x)) {}

judgement conclusion_of(derivation const& x)
{
    struct visitor
    {
        judgement operator()(derivation::var_t const& x) const
        {
            return {x.ctx(), statement(pre_typed_term(x.var()), x.ty())};
        }

        judgement operator()(derivation::app_t const& x) const
        {
            return judgement(
                x.ctx(),
                statement(
                    pre_typed_term::app(
                        conclusion_of(x.fun()).stm.subject,
                        conclusion_of(x.arg()).stm.subject),
                    x.ty()));
        }

        judgement operator()(derivation::abs_t const& x)
        {
            return judgement(
                x.ctx(),
                statement(
                    pre_typed_term::abs(x.var(), x.var_type(), conclusion_of(x.body()).stm.subject),
                    x.ty()));
        }
    };
    return std::visit(visitor{}, x.value);
}

type type_of(derivation const& d)
{
    return std::visit([] (auto const& d) { return d.ty(); }, d.value);
}

struct derivation_rules
{
    static derivation var(context ctx, pre_typed_term::var_t x, type t)
    {
        return derivation(derivation::var_t(std::move(ctx), std::move(x), std::move(t)));
    }

    static derivation app(context ctx, derivation fun, derivation arg)
    {
        auto const fun_type = type_of(fun);
        auto img_type = std::get<type::arr_t>(fun_type.value).img.get();
        return derivation(derivation::app_t(std::move(ctx), std::move(fun), std::move(arg), std::move(img_type)));
    }

    static derivation abs(context ctx, pre_typed_term::var_t var, type var_type, derivation body)
    {
        auto ty = type::arr(var_type, type_of(body));
        return derivation(derivation::abs_t(std::move(ctx), std::move(var), std::move(var_type), std::move(body), ty));
    }
};

std::optional<derivation> type_assign(context const& ctx, pre_typed_term const& x)
{
    struct visitor
    {
        context const& ctx;

        std::optional<derivation> operator()(pre_typed_term::var_t const& x) const
        {
            if (auto ty = ctx[x])
                return derivation_rules::var(ctx, x, std::move(*ty));
            else
                return std::nullopt;
        }

        std::optional<derivation> operator()(pre_typed_term::app_t const& x) const
        {
            if (auto left_d = type_assign(ctx, x.left.get()))
                if (auto const left_type = type_of(*left_d); is_arr(left_type))
                    if (auto right_d = type_assign(ctx, x.right.get()))
                        if (std::get<type::arr_t>(left_type.value).dom.get() == type_of(*right_d))
                            return derivation_rules::app(ctx, std::move(*left_d), std::move(*right_d));
            return std::nullopt;
        }

        std::optional<derivation> operator()(pre_typed_term::abs_t const& x) const
        {
            auto ext_ctx = ctx;
            ext_ctx.decls[x.var] = x.var_type;
            if (auto body_derivation = type_assign(ext_ctx, x.body.get()))
                return derivation_rules::abs(ctx, x.var, x.var_type, std::move(*body_derivation));
            else
                return std::nullopt;
        }
    };
    return std::visit(visitor{ctx}, x.value);
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
        if (auto v = term_search_impl(ctx, t, state))
            result.push_back(std::move(*v));
        else
            return {};
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

static std::string generate_fresh_var_name(context const& ctx)
{
    for (auto const& name: core::var_name_generator())
        if (not ctx.decls.contains(pre_typed_term::var_t(name)))
            return name;
    __builtin_unreachable();
}

std::optional<derivation> term_search_impl(context const& ctx, type const& target, term_search_state& state)
{
    // We might have already found a term of this type.
    auto const cache_lookup_strategy = [&] () -> std::optional<derivation>
    {
        auto const it = std::ranges::find_if(state.found, [&] (auto const& p) { return p.first == target; });
        if (it != state.found.end())
            return it->second;
        return std::nullopt;
    };

    // The easy case is if there is a declaration in the given context for the target type.
    auto const var_rule_strategy = [&] () -> std::optional<derivation>
    {
        for (auto const& [var, ty]: ctx.decls)
            if (target == ty)
                return derivation_rules::var(ctx, var, target);
        return std::nullopt;
    };
    // There isn't one. But there might be a function whose image type matches with the target type.
    // If so, it suffices to find terms in the domain types (or a subset in case of partial application).
    auto const first_order_application_strategy = [&] () -> std::optional<derivation>
    {
        for (auto const& [var, ty]: ctx.decls)
            if (auto const* const p_arr_type = std::get_if<type::arr_t>(&ty.value))
            {
                auto const arg_terms = find_all_or_nothing(ctx, try_split_fun_args(*p_arr_type, target), state);
                if (not arg_terms.empty())
                {
                    auto const first_arg = arg_terms.begin();
                    return std::accumulate(
                        std::next(first_arg), arg_terms.end(),
                        derivation_rules::app(ctx, derivation_rules::var(ctx, var, ty), *first_arg),
                        [&ctx] (derivation const& acc, derivation const& arg)
                        {
                            return derivation_rules::app(ctx, acc, arg);
                        });
                }
            }
            return std::nullopt;
    };
    // If we are looking for a term of a function type, we might be able to assume the argument and deduce the result.
    // NOTE: we are going to perform a new search, but in a new context, so we start afresh with a new state.
    // TODO: by Thinning Lemma, all terms we already found in the old context can be found in the new extended context,
    // so we could in principle re-use the terms we already found.
    auto const first_order_abstraction_strategy = [&] () -> std::optional<derivation>
    {
        struct visitor
        {
            context const& ctx;

            std::optional<derivation> operator()(type::var_t const& target) const { return std::nullopt; }

            std::optional<derivation> operator()(type::arr_t const& target) const
            {
                auto new_var = pre_typed_term::var_t(generate_fresh_var_name(ctx));
                auto ext_ctx = ctx;
                ext_ctx.decls.emplace(new_var, target.dom.get());
                if (auto d = term_search(std::move(ext_ctx), target.img.get()))
                    return derivation_rules::abs(ctx, std::move(new_var), target.dom.get(), std::move(*d));
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
        first_order_abstraction_strategy
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
