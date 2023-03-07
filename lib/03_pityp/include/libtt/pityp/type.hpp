#pragma once

#include <boost/variant/recursive_wrapper.hpp>

#include <compare>
#include <string>
#include <variant>
#include <set>

namespace libtt::pityp {

struct type
{
    using rec_t = boost::recursive_wrapper<type>;

    struct var_t
    {
        std::string name;

        auto operator<=>(var_t const&) const = default;
    };

    struct arr_t
    {
        rec_t dom;
        rec_t img;

        bool operator==(arr_t const&) const;
    };

    struct pi_t
    {
        var_t var;
        rec_t body;

        bool operator==(pi_t const&) const;
    };

    using value_t = std::variant<var_t, arr_t, pi_t>;

    value_t value;

    bool operator==(type const&) const = default;

    static type var(std::string name);
    static type arr(type, type);
    static type pi(std::string var_name, type);
    static type pi(var_t, type);
};

inline bool is_var(type const& x) { return std::holds_alternative<type::var_t>(x.value); }
inline bool is_arr(type const& x) { return std::holds_alternative<type::arr_t>(x.value); }
inline bool is_pi(type const& x) { return std::holds_alternative<type::pi_t>(x.value); }

std::set<type::var_t> free_type_vars(type const&);
std::set<type::var_t> binding_and_free_type_vars(type const&);

type::pi_t rename(type::pi_t const&, std::set<type::var_t> const& forbidden_names = {});
type replace(type const&, type::var_t const& from, type::var_t const& to);
type substitute(type const&, type::var_t const&, type const&);

bool is_alpha_equivalent(type const&, type const&);

}
