#include "libtt/untyp/beta_reduction.hpp"

#include "libtt/untyp/alpha_equivalence.hpp"
#include "libtt/untyp/substitute.hpp"
#include "libtt/untyp/subterms.hpp"

#include <algorithm>
#include <list>

namespace libtt::untyp {

// is_redex

static bool is_redex(term::var_t const&)
{
    return false;
}

static bool is_redex(term::app_t const& x)
{
    return is_abs(x.left.get());
}

static bool is_redex(term::abs_t const&)
{
    return false;
}

bool is_redex(term const& x)
{
    return std::visit([] (auto const& x) { return is_redex(x); }, x.value);
}

// num_redexes

static std::size_t num_redexes(term::var_t const& x)
{
    return 0ul;
}

static std::size_t num_redexes(term::app_t const& x)
{ 
    return num_redexes(x.left.get()) + num_redexes(x.right.get()) + (is_redex(x) ? 1 : 0);
}

static std::size_t num_redexes(term::abs_t const& x)
{
    return num_redexes(x.body.get());
}

std::size_t num_redexes(term const& x)
{
    return std::visit([] (auto const& x) { return num_redexes(x); }, x.value);
}

// one_step_beta_reductions

static std::vector<term> one_step_beta_reductions(term::var_t const& x)
{
    return {};
}

static std::vector<term> one_step_beta_reductions(term::app_t const& x)
{
    std::vector<term> result;
    if (auto const* const p_abs = std::get_if<term::abs_t>(&x.left.get().value))
        result.push_back(substitute(p_abs->body.get(), p_abs->var, x.right.get()));
    std::ranges::transform(
        one_step_beta_reductions(x.left.get()),
        std::back_inserter(result),
        [&x] (term& r) { return term::app(std::move(r), x.right.get()); }
    );
    std::ranges::transform(
        one_step_beta_reductions(x.right.get()),
        std::back_inserter(result),
        [&x] (term& r) { return term::app(x.left.get(), std::move(r)); }
    );
    return result;
}

static std::vector<term> one_step_beta_reductions(term::abs_t const& x)
{
    std::vector<term> result;
    std::ranges::transform(
        one_step_beta_reductions(x.body.get()),
        std::back_inserter(result),
        [&x] (term& r) { return term::abs(x.var, std::move(r)); }
    );
    return result;
}

std::vector<term> one_step_beta_reductions(term const& x)
{
    return std::visit([] (auto const& x) { return one_step_beta_reductions(x); }, x.value);
}

// beta_normalize

static std::optional<term> beta_normalize(term::var_t const& x)
{
    return term(x);
}

static std::optional<term> beta_normalize(term::app_t const& x)
{
    auto const orig = term(x);
    if (is_beta_normal(orig))
        return orig;
    std::list<term> all_possible_reductions{orig};
    auto const append = [&all_possible_reductions] (term&& r) { all_possible_reductions.push_back(std::move(r)); };
    do
    {
        auto& curr = all_possible_reductions.front();
        auto const curr_redexes = num_redexes(curr);
        for (auto& r: one_step_beta_reductions(curr))
        {
            auto const new_redexes = num_redexes(r);
            if (new_redexes == 0ul)
                return std::move(r);
            else if (new_redexes < curr_redexes)
                append(std::move(r));
            else if (is_subterm_of(curr, r) or is_subterm_of(orig, r))
                continue; // discard reductions along an infinite path
            else
                append(std::move(r));
        }
        all_possible_reductions.pop_front();
    } while(not all_possible_reductions.empty());
    return std::nullopt;
}

static std::optional<term> beta_normalize(term::abs_t const& x)
{
    if (num_redexes(x) == 0ul)
        return term(x);
    else if (auto reduced_body = beta_normalize(x.body.get()))
        return term::abs(x.var, std::move(*reduced_body));
    else
        return std::nullopt;
}

std::optional<term> beta_normalize(term const& x)
{
    return std::visit([] (auto const& x) { return beta_normalize(x); }, x.value);
}

boost::logic::tribool beta_reduces_to(term const& x, term const& y)
{
    bool has_infinite_recursion_path = false;
    std::list<term> candidates{x};
    do
    {
        if (is_alpha_equivalent(candidates.front(), y))
            return true;
        for (auto& r: one_step_beta_reductions(candidates.front()))
        {
            if (not is_subterm_of(candidates.front(), r))
                candidates.push_back(std::move(r));
            else
                has_infinite_recursion_path = true;
        }
        candidates.pop_front();
    } while (not candidates.empty());
    if (has_infinite_recursion_path)
        return boost::logic::indeterminate;
    else
        return false;
}

boost::logic::tribool is_beta_equivalent(term const& x, term const& y)
{
    if (is_alpha_equivalent(x, y))
        return true;
    auto const& reduct_x = beta_normalize(x);
    auto const& reduct_y = beta_normalize(y);
    if (reduct_x and reduct_y)
        return *reduct_x == *reduct_y;
    if (reduct_x.has_value() != reduct_y.has_value())
        return false;
    // at this point we know that both contain an infinite reduction path; the best we can do is apply some heuristics
    if (beta_reduces_to(x, y) or beta_reduces_to(y, x))
        // for instance, one might reduce to the other
        return true;
    // TODO: add more heuristics...
    return boost::logic::indeterminate;
}

}

