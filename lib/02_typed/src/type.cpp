#include "libtt/typed/type.hpp"

#include <tuple>

namespace libtt::typed {

bool type::arr_t::operator==(type::arr_t const& that) const
{
    return std::tie(dom.get(), img.get()) == std::tie(that.dom.get(), that.img.get());
}

type type::var(std::string name)
{
    return type(type::var_t(std::move(name)));
}

type type::arr(type dom, type img)
{
    return type(type::arr_t(std::move(dom), std::move(img)));
}

}
