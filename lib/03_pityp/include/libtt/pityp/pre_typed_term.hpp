#pragma once

#include "libtt/pityp/type.hpp"

#include <boost/variant/recursive_wrapper.hpp>

#include <compare>
#include <string>
#include <variant>

namespace libtt::pityp {

struct pre_typed_term
{
    using rec_t = boost::recursive_wrapper<pre_typed_term>;

    struct var_t
    {
        std::string name;
        auto operator<=>(var_t const&) const = default;
    };

    struct app1_t // first order application
    {
        rec_t left;
        rec_t right;

        bool operator==(app1_t const&) const;
    };

    struct app2_t // second order application
    {
        rec_t left;
        type right;

        bool operator==(app2_t const&) const;
    };

    struct abs1_t // first order abstraction
    {
        var_t var;
        type var_type;
        rec_t body;

        bool operator==(abs1_t const&) const;
    };

    struct abs2_t // second order abstraction
    {
        type::var_t var;
        rec_t body;

        bool operator==(abs2_t const&) const;
    };

    using value_t = std::variant<var_t, app1_t, app2_t, abs1_t, abs2_t>;

    value_t value;

    bool operator==(pre_typed_term const&) const = default;

    static pre_typed_term var(std::string);
    static pre_typed_term app(pre_typed_term, pre_typed_term);
    static pre_typed_term app(pre_typed_term, type);
    static pre_typed_term abs(var_t, type, pre_typed_term);
    static pre_typed_term abs(type::var_t, pre_typed_term);
};

inline bool is_var(pre_typed_term const& x) { return std::holds_alternative<pre_typed_term::var_t>(x.value); }
inline bool is_app1(pre_typed_term const& x) { return std::holds_alternative<pre_typed_term::app1_t>(x.value); }
inline bool is_app2(pre_typed_term const& x) { return std::holds_alternative<pre_typed_term::app2_t>(x.value); }
inline bool is_abs1(pre_typed_term const& x) { return std::holds_alternative<pre_typed_term::abs1_t>(x.value); }
inline bool is_abs2(pre_typed_term const& x) { return std::holds_alternative<pre_typed_term::abs2_t>(x.value); }

std::set<pre_typed_term::var_t> free_variables(pre_typed_term const&);
std::set<pre_typed_term::var_t> binding_variables(pre_typed_term const&);
std::set<pre_typed_term::var_t> binding_and_free_variables(pre_typed_term const&);

pre_typed_term::abs1_t rename(pre_typed_term::abs1_t const&, std::set<pre_typed_term::var_t> const& forbidden_names = {});
pre_typed_term::abs2_t rename(pre_typed_term::abs2_t const&, std::set<type::var_t> const& forbidden_names = {});
pre_typed_term replace(pre_typed_term const&, pre_typed_term::var_t const& from, pre_typed_term::var_t const& to);
pre_typed_term substitute(pre_typed_term const& x, pre_typed_term::var_t const& var, pre_typed_term const& y);
pre_typed_term substitute(pre_typed_term const& x, type::var_t const&, type const&);

}
