#include "libtt/pityp/pretty_print.hpp"

namespace libtt::pityp {

namespace {

template <typename T>
std::ostream& with_parens(std::ostream& os, T const& x)
{
    return pretty_print(os << '(', x) << ')';
}

static std::ostream& print_var_decl(std::ostream& os, pre_typed_term::var_t const& var, type const& ty)
{
    os << var.name << " : ";
    if (is_pi(ty))
        with_parens(os, ty);
    else
        pretty_print(os, ty);
    return os;
}

struct pretty_print_visitor
{
    std::ostream& os;

    std::ostream& operator()(pre_typed_term::var_t const& x) const
    {
        return os << x.name;
    }

    std::ostream& operator()(pre_typed_term::app1_t const& x) const
    {
        print_app_term(x.left.get());
        if (is_var(x.left.get()) and is_var(x.right.get()))
            os << ' ';
        print_app_term(x.right.get());
        return os;
    }

    std::ostream& operator()(pre_typed_term::app2_t const& x) const
    {
        print_app_term(x.left.get());
        if (is_var(x.left.get()) and is_var(x.right))
            os << ' ';
        print_app_term(x.right);
        return os;
    }

    std::ostream& operator()(pre_typed_term::abs1_t const& x) const
    {
        print_var_decl(os << "lambda ", x.var, x.var_type);
        auto const &body = print_all_args(x);
        return pretty_print(os << " . ", body);
    }

    std::ostream& operator()(pre_typed_term::abs2_t const& x) const
    {
        os << "lambda " << x.var.name << " : *";
        auto const &body = print_all_args(x);
        return pretty_print(os << " . ", body);
    }

private:
    template <typename T>
    std::ostream& print_app_term(T const& x) const
    {
        return is_var(x) ? pretty_print(os, x) : with_parens(os, x);
    }

    template <typename T>
    requires (std::is_same_v<T, pre_typed_term::abs1_t> or std::is_same_v<T, pre_typed_term::abs2_t>)
    pre_typed_term const& print_all_args(T const& x) const
    {
        auto const* body_ptr = &x.body;
        while (is_abs1(body_ptr->get()) or is_abs2(body_ptr->get()))
        {
            if (auto const* const p = std::get_if<pre_typed_term::abs1_t>(&body_ptr->get().value))
            {
                print_var_decl(os << ", ", p->var, p->var_type);
                body_ptr = &p->body;
            }
            else
            {
                auto const& abs2 = std::get<pre_typed_term::abs2_t>(body_ptr->get().value);
                os << ", " << abs2.var.name << " : *";
                body_ptr = &abs2.body;
            }
        }
        return body_ptr->get();
    }
};

} // anon namespace

//static std::ostream& pretty_print(std::ostream& os, pre_typed_term::var_t const& x, type const& t)
//{
//    return pretty_print(os << x.name << " : ", t);
//}
//
//static std::ostream& pretty_print(std::ostream& os, pre_typed_term const& x, type const& t)
//{
//    if (is_var(x))
//        pretty_print(os, x);
//    else
//    {
//        os << '(';
//        pretty_print(os, x);
//        os << ')';
//    }
//    return pretty_print(os << " : ", t);
//}
//
// std::ostream& pretty_print(std::ostream& os, judgement const& x)
// {
//     os << '{';
//     bool comma = false;
//     for (auto const& decl : x.ctx.decls)
//         pretty_print(std::exchange(comma, true) ? os << ", " : os, decl.first, decl.second);
//     return pretty_print(os << "} => ", x.stm);
// }

std::ostream& pretty_print(std::ostream& os, pre_typed_term const& x)
{
    return std::visit(pretty_print_visitor{os}, x.value);
}

// std::ostream& pretty_print(std::ostream& os, statement const& x)
// {
//     if (is_var(x.subject))
//         pretty_print(os, x.subject);
//     else
//     {
//         os << '(';
//         pretty_print(os, x.subject);
//         os << ')';
//     }
//     return pretty_print(os << " : ", x.ty);
// }

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

        std::ostream& operator()(type::pi_t const& x) const
        {
            os << "pi " << x.var.name;
            auto const* body_ptr = &x.body;
            while (auto const* const p = std::get_if<type::pi_t>(&body_ptr->get().value))
            {
                os << ", " << p->var.name;
                body_ptr = &p->body;
            }
            return pretty_print(os << " : * . ", body_ptr->get());
        }
    };
    return std::visit(visitor{os}, x.value);
}

}
