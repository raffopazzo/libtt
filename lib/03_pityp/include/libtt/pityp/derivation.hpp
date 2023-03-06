#pragma once

#include "libtt/pityp/pre_typed_term.hpp"
#include "libtt/pityp/type.hpp"

#include <map>
#include <optional>
#include <string>
#include <variant>

namespace libtt::pityp {

struct context
{
    struct type_decl_t
    {
        constexpr bool operator==(type_decl_t const&) const { return true; }
    };

    bool empty() const { return m_decls.empty(); }
    bool contains(type::var_t const&) const;
    bool contains(pre_typed_term::var_t const&) const;

    std::optional<type_decl_t> operator[](type::var_t const&) const;
    std::optional<type> operator[](pre_typed_term::var_t const&) const;

    friend std::optional<context> extend(context const&, type::var_t const&);
    friend std::optional<context> extend(context const&, pre_typed_term::var_t const&, type const&);

private:
    using var_decl_t = type; // a variable declaration is just the type it maps to; the name is the key of the map
    using decl_t = std::variant<type_decl_t, var_decl_t>;

    std::map<std::string, decl_t> m_decls;
};

}
