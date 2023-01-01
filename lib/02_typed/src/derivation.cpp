#include "libtt/typed/derivation.hpp"

namespace libtt::typed {

std::optional<type> context::operator[](pre_typed_term::var_t const& v) const
{
    auto const it = decls.find(v);
    return it == decls.end() ? std::nullopt : std::optional{it->second};
}

derivation::derivation(var_t x) : value(std::move(x)) {}
derivation::derivation(app_t x) : value(std::move(x)) {}
derivation::derivation(abs_t x) : value(std::move(x)) {}

judgement derivation::conclusion() const
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
                        x.left.get().conclusion().stm.subject,
                        x.right.get().conclusion().stm.subject
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
                    pre_typed_term::abs(x.var, x.var_type, x.body.get().conclusion().stm.subject),
                    x.ty
                )       
            );
        }
    };
    return std::visit(visitor{}, value);
}

std::optional<derivation> type_assign(context const& ctx, pre_typed_term const& x)
{
    struct visitor
    {
        context const& ctx;

        std::optional<derivation> operator()(pre_typed_term::var_t const& x) const
        {
            if (auto ty = ctx[x])
                return derivation(derivation::var_t(ctx, x, *ty));
            else
                return std::nullopt;
        }

        std::optional<derivation> operator()(pre_typed_term::app_t const& x) const
        {
            auto left_d = type_assign(ctx, x.left.get());
            if (not left_d) return std::nullopt;
            auto const left_conclusion = left_d->conclusion();
            auto p_abs = std::get_if<type::arr_t>(&left_conclusion.stm.ty.value);
            if (not p_abs)  return std::nullopt;
            auto right_d = type_assign(ctx, x.right.get());
            if (not right_d) return std::nullopt;
            auto const right_conclusion = right_d->conclusion();
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
                auto function_type = type::arr(x.var_type, body_derivation->conclusion().stm.ty);
                return derivation(derivation::abs_t(
                    ctx,
                    x.var,
                    x.var_type,
                    *std::move(body_derivation),
                    function_type
                ));
            }
            else
                return std::nullopt;
        }
    };
    return std::visit(visitor{ctx}, x.value);
}

std::optional<derivation> term_search(context const&, type const&)
{
    return std::nullopt;
}

}
