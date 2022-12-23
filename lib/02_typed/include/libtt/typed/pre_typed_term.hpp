#pragma once

#include "libtt/typed/type.hpp"

#include <boost/variant/recursive_wrapper.hpp>

#include <compare>
#include <string>
#include <variant>

namespace libtt::typed {

struct pre_typed_term
{
    using rec_t = boost::recursive_wrapper<pre_typed_term>;

    struct var_t
    {
        std::string name;

        auto operator<=>(var_t const&) const = default;
    };

    struct app_t
    {
        rec_t left;
        rec_t right;

        bool operator==(app_t const&) const;
    };

    struct abs_t
    {
        var_t var;
        type var_type;
        rec_t body;

        bool operator==(abs_t const&) const;
    };

    using value_t = std::variant<var_t, app_t, abs_t>;

    value_t value;

    bool operator==(pre_typed_term const&) const = default;

    static pre_typed_term var(std::string);
    static pre_typed_term app(pre_typed_term, pre_typed_term);
    static pre_typed_term abs(std::string, type, pre_typed_term);
    static pre_typed_term abs(var_t, type, pre_typed_term);
};

inline bool is_var(pre_typed_term const& x) { return std::holds_alternative<pre_typed_term::var_t>(x.value); }
inline bool is_app(pre_typed_term const& x) { return std::holds_alternative<pre_typed_term::app_t>(x.value); }
inline bool is_abs(pre_typed_term const& x) { return std::holds_alternative<pre_typed_term::abs_t>(x.value); }

}
