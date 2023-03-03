#include "libtt/untyp/pretty_print.hpp"

namespace libtt::untyp {

template <typename T>
std::ostream& with_parens(std::ostream& os, T const& x)
{
    return pretty_print(os << '(', x) << ')';
}

namespace detail {

struct pretty_print_visitor
{
    std::ostream& os;

    std::ostream& operator()(term::var_t const&);
    std::ostream& operator()(term::app_t const&);
    std::ostream& operator()(term::abs_t const&);
};

std::ostream& pretty_print_visitor::operator()(term::var_t const& x)
{
    return os << x.name;
}

std::ostream& pretty_print_visitor::operator()(term::app_t const& x)
{
    auto const impl = [this] (term const& y)
    {
        if (is_var(y))
            pretty_print(os, y);
        else
            with_parens(os, y);
    };
    impl(x.left.get());
    if (is_var(x.left.get()) and is_var(x.right.get()))
        os << ' ';
    impl(x.right.get());
    return os;
}

std::ostream& pretty_print_visitor::operator()(term::abs_t const& x)
{
    os << "lambda " << x.var.name;
    auto const* body_ptr = &x.body;
    while (auto const* const p = std::get_if<term::abs_t>(&body_ptr->get().value))
    {
        os << ' ' << p->var.name;
        body_ptr = &p->body;
    }
    return pretty_print(os << " . ", body_ptr->get());
}

}

std::ostream& pretty_print(std::ostream& os, term const& x)
{
    std::visit(detail::pretty_print_visitor{os}, x.value);
    return os;
}

}
