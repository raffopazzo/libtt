#include "libtt/typed/pretty_print.hpp"

namespace libtt::typed {

template <typename T>
std::ostream& with_parens(std::ostream& os, T const& x)
{
    return pretty_print(os << '(', x) << ')';
}

static std::ostream& pretty_print(std::ostream& os, pre_typed_term::var_t const& x, type const& t)
{
    return pretty_print(os << x.name << " : ", t);
}

static std::ostream& pretty_print(std::ostream& os, pre_typed_term const& x, type const& t)
{
    if (is_var(x))
        pretty_print(os, x);
    else
        with_parens(os, x);
    return pretty_print(os << " : ", t);
}

std::ostream& pretty_print(std::ostream& os, judgement const& x)
{
    os << '{';
    bool comma = false;
    for (auto const& decl : x.ctx.decls)
        pretty_print(std::exchange(comma, true) ? os << ", " : os, decl.first, decl.second);
    return pretty_print(os << "} => ", x.stm);
}

std::ostream& pretty_print(std::ostream& os, pre_typed_term const& x)
{
    struct visitor
    {
        std::ostream& os;

        std::ostream& operator()(pre_typed_term::var_t const& x) const
        {
            return os << x.name;
        }

        std::ostream& operator()(pre_typed_term::app_t const& x) const
        {
            auto const impl = [this] (pre_typed_term const& y)
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

        std::ostream& operator()(pre_typed_term::abs_t const& x) const
        {
            os << "lambda " << x.var.name;
            auto const* body_ptr = &x.body;
            pretty_print(os << " : ", x.var_type);
            while (auto const* const p = std::get_if<pre_typed_term::abs_t>(&body_ptr->get().value))
            {
                os << ", " << p->var.name;
                pretty_print(os << " : ", p->var_type);
                body_ptr = &p->body;
            }
            return pretty_print(os << " . ", body_ptr->get());
        }
    };
    return std::visit(visitor{os}, x.value);
}

std::ostream& pretty_print(std::ostream& os, statement const& x)
{
    if (is_var(x.subject))
        pretty_print(os, x.subject);
    else
        with_parens(os, x.subject);
    return pretty_print(os << " : ", x.ty);
}

std::ostream& pretty_print(std::ostream& os, type const& x)
{
    struct visitor
    {
        std::ostream& os;

        std::ostream& operator()(type::var_t const& x) const
        {
            return os << x.name;
        }

        std::ostream& operator()(type::arr_t const& x) const
        {
            if (is_var(x.dom.get()))
                pretty_print(os, x.dom.get());
            else
                with_parens(os, x.dom.get());
            os << " -> ";
            return pretty_print(os, x.img.get());
        }
    };
    return std::visit(visitor{os}, x.value);
}

}
