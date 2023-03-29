#include "libtt/pityp/pretty_print.hpp"

namespace libtt::pityp {

namespace {

template <typename T>
std::ostream& pretty_print(std::ostream& os, std::vector<T> const& xs)
{
    bool comma = false;
    for (auto const& x: xs)
        pretty_print(std::exchange(comma, true) ? os << ", " : os, x);
    return os;
}

template <typename T>
std::ostream& with_parens(std::ostream& os, T const& x, std::pair<char, char> parens={'(', ')'})
{
    return pretty_print(os << parens.first, x) << parens.second;
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

// context

std::ostream& pretty_print(std::ostream& os, context const& ctx)
{
    return with_parens(os, decls_of(ctx), {'{', '}'});
}

std::ostream& pretty_print(std::ostream& os, context::decl_t const& x)
{
    return std::visit([&os] (auto const& v) -> std::ostream& { return pretty_print(os, v); }, x);
}

std::ostream& pretty_print(std::ostream& os, context::type_decl_t const& x)
{
    return os << x.subject.name << " : *";
}

std::ostream& pretty_print(std::ostream& os, context::var_decl_t const& x)
{
    os << x.subject.name << " : ";
    return is_var(x.ty) ? pretty_print(os, x.ty) : with_parens(os, x.ty);
}

// judgement

std::ostream& pretty_print(std::ostream& os, judgement<term_stm_t> const& x)
{
    pretty_print(os, x.ctx) << " => ";
    if (is_var(x.stm.subject))
        pretty_print(os, x.stm.subject);
    else
        with_parens(os, x.stm.subject);
    return pretty_print(os << " : ", x.stm.ty);
}

std::ostream& pretty_print(std::ostream& os, legal_term const& x)
{
    pretty_print(os, x.ctx());
    pretty_print(os << " => ", x.term());
    pretty_print(os << " : ", x.ty());
    return os;
}

// pre_typed_term

std::ostream& pretty_print(std::ostream& os, pre_typed_term const& x)
{
    return std::visit(pretty_print_visitor{os}, x.value);
}

// type

std::ostream& pretty_print(std::ostream& os, type const& x)
{
    return std::visit([&os] (auto const& v) -> std::ostream& { return pretty_print(os, v); }, x.value);
}

std::ostream& pretty_print(std::ostream& os, type::var_t const& x)
{
    return os << x.name;
}

std::ostream& pretty_print(std::ostream& os, type::arr_t const& x)
{
    if (is_var(x.dom.get()))
        pretty_print(os, x.dom.get());
    else
        with_parens(os, x.dom.get());
    os << " -> ";
    return pretty_print(os, x.img.get());
}

std::ostream& pretty_print(std::ostream& os, type::pi_t const& x)
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

}
