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

term term::abs(var_t name, term body)
{
    return term(abs_t(std::move(name), std::move(body)));
}

bool is_var(term const& x) { return std::holds_alternative<term::var_t>(x.value); }
bool is_app(term const& x) { return std::holds_alternative<term::app_t>(x.value); }
bool is_abs(term const& x) { return std::holds_alternative<term::abs_t>(x.value); }

bool is_closed(term const& x) { return free_variables(x).empty(); }

std::set<term::var_t> free_variables(term const& x)
{
    struct visitor
    {
        std::set<term::var_t> operator()(term::var_t const& x) const { return {x}; }
        std::set<term::var_t> operator()(term::app_t const& x) const
        {
            auto left_set = free_variables(x.left.get());
            auto right_set = free_variables(x.right.get());
            left_set.insert(std::make_move_iterator(right_set.begin()), std::make_move_iterator(right_set.end()));
            return left_set;
        }
        std::set<term::var_t> operator()(term::abs_t const& x) const
        {
            auto tmp = free_variables(x.body.get());
            tmp.erase(x.var);
            return tmp;
        }
    };
    return std::visit(visitor{}, x.value);
}

std::set<term::var_t> binding_variables(term const& x)
{
    struct visitor
    {
        std::set<term::var_t> operator()(term::var_t const& x) const { return {}; }
        std::set<term::var_t> operator()(term::app_t const& x) const
        {
            auto left_set = binding_variables(x.left.get());
            auto right_set = binding_variables(x.right.get());
            left_set.insert(std::make_move_iterator(right_set.begin()), std::make_move_iterator(right_set.end()));
            return left_set;
        }
        std::set<term::var_t> operator()(term::abs_t const& x)
        {
            auto tmp = binding_variables(x.body.get());
            tmp.insert(x.var);
            return tmp;
        }
    };
    return std::visit(visitor{}, x.value);
}

std::set<term::var_t> binding_and_free_variables(term const& x)
{
    std::set<term::var_t> result;
    auto inserter = std::inserter(result, result.end());
    std::ranges::move(free_variables(x), inserter);
    std::ranges::move(binding_variables(x), inserter);
    return result;
}

}
