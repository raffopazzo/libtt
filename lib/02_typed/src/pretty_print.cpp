#include "libtt/typed/pretty_print.hpp"

namespace libtt::typed {

static std::ostream& pretty_print(std::ostream& os, pre_typed_term::var_t const& x, type const& t)
{
    return pretty_print(os << x.name << " : ", t);
}

static std::ostream& pretty_print(std::ostream& os, pre_typed_term const& x, type const& t)
{
    if (is_var(x))
        pretty_print(os, x);
    else
    {
        os << '(';
        pretty_print(os, x);
        os << ')';
    }
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
                {
                    os << '(';
                    pretty_print(os, y);
                    os << ')';
                }
            };
            impl(x.left.get());
            if (is_var(x.left.get()) and is_var(x.right.get()))
                os << ' ';
            impl(x.right.get());
            return os;
        }

        std::ostream& operator()(pre_typed_term::abs_t const& x) const
        {
            os << "lambda";
            auto const* body_ptr = &x.body;
            os << ' ' << x.var.name;
            pretty_print(os << " : ", x.var_type);
            while (auto const* const p = std::get_if<pre_typed_term::abs_t>(&body_ptr->get().value))
            {
                os << ", " << p->var.name;
                pretty_print(os << " : ", p->var_type);
                body_ptr = &p->body;
            }
            os << " . ";
            return pretty_print(os, body_ptr->get());
        }
    };
    return std::visit(visitor{os}, x.value);
}

std::ostream& pretty_print(std::ostream& os, statement const& x)
{
    if (is_var(x.subject))
        pretty_print(os, x.subject);
    else
    {
        os << '(';
        pretty_print(os, x.subject);
        os << ')';
    }
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
                pretty_print(os << "(", x.dom.get()) << ")";
            os << " -> ";
            return pretty_print(os, x.img.get());
        }
    };
    return std::visit(visitor{os}, x.value);
}

}
