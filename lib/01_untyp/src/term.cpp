#include "libtt/untyp/term.hpp"

#include <algorithm>

namespace libtt::untyp {

bool term::app_t::operator==(term::app_t const& that) const
{
    return left.get() == that.left.get() && right.get() == that.right.get();
}

bool term::abs_t::operator==(term::abs_t const& that) const
{
    return var == that.var and body.get() == that.body.get();
}

term term::var(std::string name)
{
    return term(var_t(std::move(name)));
}

term term::app(term x, term y)
{
    return term(app_t(std::move(x), std::move(y)));
}

term term::abs(std::string var_name, term body)
{
    return term(abs_t(var_t(std::move(var_name)), std::move(body)));
}

bool is_var(term const& x) { return std::holds_alternative<term::var_t>(x.value); }
bool is_app(term const& x) { return std::holds_alternative<term::app_t>(x.value); }
bool is_abs(term const& x) { return std::holds_alternative<term::abs_t>(x.value); }

namespace {

std::set<term::var_t> free_variables(term::var_t const& x) { return {x}; }
std::set<term::var_t> free_variables(term::app_t const& x)
{
    auto left_set = free_variables(x.left.get());
    auto right_set = free_variables(x.right.get());
    left_set.insert(std::make_move_iterator(right_set.begin()), std::make_move_iterator(right_set.end()));
    return left_set;
}
std::set<term::var_t> free_variables(term::abs_t const& x)
{
    auto tmp = free_variables(x.body.get());
    tmp.erase(x.var);
    return tmp;
}

}

std::set<term::var_t> free_variables(term const& x)
{
    return std::visit([] (auto const& t) { return free_variables(t); }, x.value);
}

}
