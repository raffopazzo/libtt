#include "libtt/typed/pretty_print.hpp"

namespace libtt::typed {

std::ostream& pretty_print(std::ostream& os, type const& x)
{
    struct visitor
    {
        std::ostream& os;

        std::ostream& operator()(type::var_t const& x)
        {
            return os << x.name;
        }

        std::ostream& operator()(type::arr_t const& x)
        {
            if (is_var(x.dom.get()))
                pretty_print(os, x.dom.get());
            else
                pretty_print(os << "(", x.dom.get()) << ")";
            os << " -> ";
            return pretty_print(os, x.img.get());
        }
    };
    return std::visit(visitor{os}, x.value);
}

}
