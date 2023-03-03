#include "libtt/pityp/pre_typed_term.hpp"

#include <tuple>

namespace libtt::pityp {

bool pre_typed_term::app1_t::operator==(app1_t const& that) const
{
    return std::tie(left.get(), right.get()) == std::tie(that.left.get(), that.right.get());
}

bool pre_typed_term::app2_t::operator==(app2_t const& that) const
{
    return std::tie(left.get(), right) == std::tie(that.left.get(), that.right);
}

bool pre_typed_term::abs1_t::operator==(abs1_t const& that) const
{
    return std::tie(var, var_type, body.get()) == std::tie(that.var, that.var_type, that.body.get());
}

bool pre_typed_term::abs2_t::operator==(abs2_t const& that) const
{
    return std::tie(var, body.get()) == std::tie(that.var, that.body.get());
}

pre_typed_term pre_typed_term::var(std::string name)
{
    return pre_typed_term(var_t(std::move(name)));
}

pre_typed_term pre_typed_term::app(pre_typed_term left, pre_typed_term right)
{
    return pre_typed_term(app1_t(std::move(left), std::move(right)));
}

pre_typed_term pre_typed_term::app(pre_typed_term left, type right)
{
    return pre_typed_term(app2_t(std::move(left), std::move(right)));
}

pre_typed_term pre_typed_term::abs(var_t var_name, type var_type, pre_typed_term body)
{
    return pre_typed_term(abs1_t(std::move(var_name), std::move(var_type), std::move(body)));
}

pre_typed_term pre_typed_term::abs(type::var_t type_var, pre_typed_term body)
{
    return pre_typed_term(abs2_t(std::move(type_var), std::move(body)));
}

}
