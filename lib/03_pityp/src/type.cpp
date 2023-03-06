#include "libtt/pityp/type.hpp"

#include <algorithm>
#include <iterator>
#include <tuple>

namespace libtt::pityp {

bool type::arr_t::operator==(type::arr_t const& that) const
{
    return std::tie(dom.get(), img.get()) == std::tie(that.dom.get(), that.img.get());
}

bool type::pi_t::operator==(type::pi_t const& that) const
{
    return std::tie(var, body.get()) == std::tie(that.var, that.body.get());
}

type type::var(std::string name)
{
    return type(type::var_t(std::move(name)));
}

type type::arr(type dom, type img)
{
    return type(type::arr_t(std::move(dom), std::move(img)));
}

type type::pi(std::string var_name, type body)
{
    return type::pi(type::var_t(std::move(var_name)), std::move(body));
}

type type::pi(type::var_t var, type body)
{
    return type(type::pi_t(std::move(var), std::move(body)));
}

std::set<type::var_t> free_type_vars(type const& ty)
{
    struct visitor
    {
        std::set<type::var_t> result;

        void operator()(type::var_t const& x)
        {
            result.insert(x);
        }

        void operator()(type::arr_t const& x)
        {
            visit(x.dom.get());
            visit(x.img.get());
        }

        void operator()(type::pi_t const& x)
        {
            auto tmp = free_type_vars(x.body.get());
            tmp.erase(x.var);
            std::ranges::move(tmp, std::inserter(result, result.end()));
        }

        void visit(type const& x)
        {
            std::visit(*this, x.value);
        }
    };

    visitor v;
    v.visit(ty);
    return v.result;
}

}
