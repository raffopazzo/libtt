#pragma once

#include <boost/variant/recursive_wrapper.hpp>

#include <compare>
#include <optional>
#include <set>
#include <string>
#include <variant>

namespace libtt::untyp {

struct term
{
    using rec_t = boost::recursive_wrapper<term>;

    struct var_t
    {
        std::string name;

        auto operator<=>(var_t const&) const = default;
    };

    struct app_t
    {
        rec_t left;
        rec_t right;

        bool operator==(app_t const& that) const;
    };

    struct abs_t
    {
        var_t var;
        rec_t body;

        bool operator==(abs_t const& that) const;
    };

    using value_t = std::variant<var_t, app_t, abs_t>;

    value_t value;

    bool operator==(term const&) const = default;

    static term var(std::string);
    static term app(term, term);
    static term abs(std::string, term);
    static term abs(var_t, term);
};

inline bool is_var(term const& x) { return std::holds_alternative<term::var_t>(x.value); }
inline bool is_app(term const& x) { return std::holds_alternative<term::app_t>(x.value); }
inline bool is_abs(term const& x) { return std::holds_alternative<term::abs_t>(x.value); }

std::set<term::var_t> free_variables(term const&);
std::set<term::var_t> binding_variables(term const&);
std::set<term::var_t> binding_and_free_variables(term const&);

inline bool is_closed(term const& x) { return free_variables(x).empty(); }

term::abs_t rename(term::abs_t const&, std::set<term::var_t> const& forbidden_names = {});

term replace(term const&, term::var_t const& from, term::var_t const& to);

term substitute(term const& x, term::var_t const& var, term const& y);

}
