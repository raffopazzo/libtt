#include "libtt/typed/derivation.hpp"
#include "libtt/core/var_name_generator.hpp"

#include <numeric>

namespace libtt::typed {

std::optional<type> context::operator[](pre_typed_term::var_t const& v) const
{
    auto const it = decls.find(v);
    return it == decls.end() ? std::nullopt : std::optional{it->second};
}

derivation::derivation(var_t x) : value(std::move(x)) {}
derivation::derivation(app_t x) : value(std::move(x)) {}
derivation::derivation(abs_t x) : value(std::move(x)) {}

judgement conclusion_of(derivation const& x)
{
    struct visitor
    {
        judgement operator()(derivation::var_t const& x) const
        {
            return {x.ctx, statement(pre_typed_term(x.var), x.ty)};
        }

        judgement operator()(derivation::app_t const& x) const
        {
            return judgement(
                x.ctx,
                statement(
                    pre_typed_term::app(
                        conclusion_of(x.left.get()).stm.subject,
                        conclusion_of(x.right.get()).stm.subject
                    ),
                    x.ty
                )
            );
        }

        judgement operator()(derivation::abs_t const& x)
        {
            return judgement(
                x.ctx,
                statement(
                    pre_typed_term::abs(x.var, x.var_type, conclusion_of(x.body.get()).stm.subject),
                    x.ty
                )       
            );
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
            if (auto ty = ctx[x])
                return derivation(derivation::var_t(ctx, x, std::move(*ty)));
            else
                return std::nullopt;
        }

        std::optional<derivation> operator()(pre_typed_term::app_t const& x) const
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
            return derivation(derivation::app_t(
                ctx,
                std::move(*left_d),
                std::move(*right_d),
                p_abs->img.get()
            ));
        }

        std::optional<derivation> operator()(pre_typed_term::abs_t const& x) const
        {
            auto ext_ctx = ctx;
            ext_ctx.decls[x.var] = x.var_type;
            if (auto body_derivation = type_assign(ext_ctx, x.body.get()))
            {
                auto function_type = type::arr(x.var_type, conclusion_of(*body_derivation).stm.ty);
                return derivation(derivation::abs_t(
                    ctx,
                    x.var,
                    x.var_type,
                    std::move(*body_derivation),
                    function_type
                ));
            }
            else
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

std::optional<derivation> term_search(context const& ctx, type const& target)
{
    // The easy case is if there is a declaration in the given context for the target type.
    for (auto const& [var, ty]: ctx.decls)
        if (target == ty)
            return derivation(derivation::var_t(ctx, var, target));
    // There isn't one. But there might be a function whose image type matches with the target type.
    // If so, it suffices to find terms in the domain types (or a subset in case of partial application).
    for (auto const& [var, ty]: ctx.decls)
        if (auto const* const p_arr_type = std::get_if<type::arr_t>(&ty.value))
        {
            auto const arg_terms = find_all_or_nothing(ctx, try_split_fun_args(*p_arr_type, target));
            if (not arg_terms.empty())
            {
                auto const first_arg = arg_terms.begin();
                return std::accumulate(
                    std::next(first_arg), arg_terms.end(),
                    derivation(
                        derivation::app_t(
                            ctx,
                            derivation(derivation::var_t(ctx, var, ty)),
                            *first_arg,
                            p_arr_type->img.get())),
                    [&ctx] (derivation const& acc, derivation const& arg)
                    {
                        // Because we are accumulating over multi-variate function, acc must also be of arrow type,
                        // and the result of this application will be of the type of the image.
                        // Also, taking copy because otherwise, std::get would introduce a dangling reference
                        // to the temporary judgement from conclusion_of(acc).
                        auto const arr = std::get<type::arr_t>(conclusion_of(acc).stm.ty.value);
                        return derivation(derivation::app_t(ctx, acc, arg, arr.img.get()));
                    });
            }
        }
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
            auto [ext_ctx, new_var] = [this, &target] ()
            {
                for (auto new_var_name: core::var_name_generator())
                {
                    auto new_var = pre_typed_term::var_t(std::move(new_var_name));
                    if (not ctx.decls.contains(new_var))
                    {
                        auto ext_ctx = ctx;
                        ext_ctx.decls.emplace(new_var, target.dom.get());
                        return std::make_pair(std::move(ext_ctx), std::move(new_var));
                    }
                }
                __builtin_unreachable();
            }();
            if (auto d = term_search(std::move(ext_ctx), target.img.get()))
                return derivation(
                    derivation::abs_t(
                        ctx,
                        std::move(new_var),
                        target.dom.get(),
                        std::move(*d),
                        type(target)));
            return std::nullopt;
        }
    };
    return std::visit(visitor{ctx}, target.value);
}

}
