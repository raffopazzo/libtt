#include "libtt/pityp/derivation.hpp"

#include <algorithm>
#include <vector>

namespace libtt::pityp {

std::optional<context> extend(context const& ctx, type::var_t const& x)
{
    std::optional<context> res;
    if (not ctx.contains(pre_typed_term::var_t(x.name)))
    {
        // Technically the definition of a context does not allow to declare a type twice,
        // but it really is an idempotent operation, so we do allow it.
        res = ctx;
        res->m_decls.emplace(x.name, context::type_decl_t{});
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
    return it != m_decls.end() and std::holds_alternative<type_decl_t>(it->second);
}

bool context::contains(pre_typed_term::var_t const& x) const
{
    auto const it = m_decls.find(x.name);
    return it != m_decls.end() and std::holds_alternative<var_decl_t>(it->second);
}

std::optional<context::type_decl_t> context::operator[](type::var_t const& x) const
{
    auto const it = m_decls.find(x.name);
    if (it != m_decls.end() and std::holds_alternative<type_decl_t>(it->second))
        return type_decl_t{};
    else
        return std::nullopt;
}

std::optional<type> context::operator[](pre_typed_term::var_t const& x) const
{
    auto const it = m_decls.find(x.name);
    if (it != m_decls.end())
        if (auto const* const p = std::get_if<type>(&it->second))
            return *p;
    return std::nullopt;
}

}
