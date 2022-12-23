#include "libtt/typed/pre_typed_term.hpp"

#include <tuple>

namespace libtt::typed {

bool pre_typed_term::app_t::operator==(app_t const& that) const
{
    return std::tie(left.get(), right.get()) == std::tie(that.left.get(), that.right.get());
}

bool pre_typed_term::abs_t::operator==(abs_t const& that) const
{
    return std::tie(var, var_type, body.get()) == std::tie(that.var, that.var_type, that.body.get());
}

pre_typed_term pre_typed_term::var(std::string name)
{
    return pre_typed_term(var_t(std::move(name)));
}

pre_typed_term pre_typed_term::app(pre_typed_term left, pre_typed_term right)
{
    return pre_typed_term(app_t(std::move(left), std::move(right)));
}

pre_typed_term pre_typed_term::abs(std::string var_name, type var_type, pre_typed_term body)
{
    return pre_typed_term(abs_t(var_t(std::move(var_name)), std::move(var_type), std::move(body)));
}

pre_typed_term pre_typed_term::abs(var_t var_name, type var_type, pre_typed_term body)
{
    return pre_typed_term(abs_t(std::move(var_name), std::move(var_type), std::move(body)));
}

}

