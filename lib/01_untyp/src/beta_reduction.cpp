#include "libtt/untyp/beta_reduction.hpp"

#include "libtt/untyp/alpha_equivalence.hpp"
#include "libtt/untyp/substitute.hpp"
#include "libtt/untyp/subterms.hpp"

#include <algorithm>

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

// beta_reduction

static std::optional<term> beta_reduction(term::var_t const& x)
{
    return term(x);
}

static std::optional<term> beta_reduction(term::app_t const& x)
{
    auto const total_redexes = num_redexes(x);
    if (total_redexes == 0ul)
        return term(x);

    auto const appears_again_inside = [x=term(x)] (term const& y)
    {
        if (is_alpha_equivalent(x, y))
            return true;
        for (auto const& s: subterms(y))
            if (is_alpha_equivalent(x, s))
                return true;
        return false;
    };

    auto all_possible_reductions = one_step_beta_reductions(x);
    // Remove all "non-viable" reductions; a reduction is "viable" if:
    //  - it reduces the total number of redexes;
    //  - or the original term does not appear again inside that reduction.
    std::erase_if(
        all_possible_reductions,
        [total_redexes, &appears_again_inside] (term const& y)
        {
            return num_redexes(y) >= total_redexes and appears_again_inside(y);
        });
    // So now we have some viable reductions; if any of them reduces to a final outcome, by Church-Rosser Theorem,
    // that is the only outcome regardless of the order in which reductions are carried out. So we can pick any of them.
    // In particular, if anything is already in beta-normal form, that is the final outcome.
    for (auto const& r: all_possible_reductions)
        if (is_beta_normal(r))
            return r;
    // Otherwise try reducing further; if anything succeeds that's the outcome.
    for (auto const& r: all_possible_reductions)
        if (auto outcome = beta_reduction(r))
            return std::move(*outcome);
    return std::nullopt;
}

static std::optional<term> beta_reduction(term::abs_t const& x)
{
    if (num_redexes(x) == 0ul)
        return term(x);
    else if (auto reduced_body = beta_reduction(x.body.get()))
        return term::abs(x.var, std::move(*reduced_body));
    else
        return std::nullopt;
}

std::optional<term> beta_reduction(term const& x)
{
    return std::visit([] (auto const& x) { return beta_reduction(x); }, x.value);
}

}
