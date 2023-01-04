#define BOOST_TEST_MODULE term_search
#include <boost/test/included/unit_test.hpp>

#include "libtt/typed/derivation.hpp"
#include "libtt/typed/pretty_print.hpp"

namespace libtt::typed {

template <typename T>
std::ostream& operator<<(std::ostream& os, std::optional<T> const& x)
{
    return x ? (os << *x) : (os << "nullopt");
}

template <typename T, typename U>
std::ostream& operator<<(std::ostream& os, std::pair<T, U> const& x)
{
    return os << x.first << ':' << x.second;
}

std::ostream& operator<<(std::ostream& os, type const& t) { return pretty_print(os, t); };

std::ostream& operator<<(std::ostream&, pre_typed_term::var_t const&);
std::ostream& operator<<(std::ostream&, pre_typed_term::app_t const&);
std::ostream& operator<<(std::ostream&, pre_typed_term::abs_t const&);
std::ostream& operator<<(std::ostream&, pre_typed_term const&);

std::ostream& operator<<(std::ostream& os, pre_typed_term::var_t const& x)
{
    return os << x.name;
}

std::ostream& operator<<(std::ostream& os, pre_typed_term::app_t const& x)
{
    return os << '(' << x.left.get() << ' ' << x.right.get() << ')';
}

std::ostream& operator<<(std::ostream& os, pre_typed_term::abs_t const& x)
{
    return os << "(lambda " << x.var << " : " << x.var_type << " . " << x.body.get() << ')';
}

std::ostream& operator<<(std::ostream& os, pre_typed_term const& x)
{
    return std::visit([&os](auto const& x) -> std::ostream& {return os << x;}, x.value);
}

std::ostream& operator<<(std::ostream& os, statement const& x)
{
    return os << x.subject << ": " << x.ty;
}

std::ostream& operator<<(std::ostream& os, judgement const& x)
{
    os << '{';
    bool comma = false;
    for (auto const& decl : x.ctx.decls)
        (std::exchange(comma, true) ? os << ", " : os) << decl;
    return os << "} => " << x.stm;
}

}

using namespace libtt::typed;

struct term_search_tests_fixture
{
    type const s = type::var("s");
    type const t = type::var("t");
    type const s_to_t = type::arr(s, t);
    type const alpha = type::var("alpha");
    type const beta = type::var("beta");
    type const gamma = type::var("gamma");
    context const empty;
};

BOOST_FIXTURE_TEST_SUITE(term_search_tests, term_search_tests_fixture)

BOOST_AUTO_TEST_CASE(var_rule_for_simple_types)
{
    context const ctx{{{pre_typed_term::var_t("x"), t}}};
    BOOST_TEST(not conclusion_of(term_search(empty, s)).has_value());
    BOOST_TEST(not conclusion_of(term_search(ctx, s)).has_value());
    BOOST_TEST(conclusion_of(term_search(ctx, t)) == judgement(ctx, statement(pre_typed_term::var("x"), t)));
}

BOOST_AUTO_TEST_CASE(var_rule_for_function)
{
    context const ctx{{{pre_typed_term::var_t("f"), s_to_t}}};
    auto const conclusion = conclusion_of(term_search(ctx, s_to_t));
    BOOST_TEST(conclusion.has_value());
    if (conclusion)
        BOOST_TEST(conclusion->stm.ty == s_to_t);
}

BOOST_AUTO_TEST_CASE(apply_function_from_context)
{
    context const ctx{{
        {pre_typed_term::var_t("f"), s_to_t},
        {pre_typed_term::var_t("u"), s}
    }};
    auto const conclusion = conclusion_of(term_search(ctx, t));
    BOOST_TEST(conclusion.has_value());
    if (conclusion)
        BOOST_TEST(conclusion->stm.ty == t);
    BOOST_TEST(conclusion == judgement(empty, statement(pre_typed_term::var("x"), t)));
}

BOOST_AUTO_TEST_CASE(exercise_2_13_a)
{
    context const ctx{{{pre_typed_term::var_t("x"), type::arr(alpha, type::arr(beta, gamma))}}};
    auto const conclusion = conclusion_of(term_search(ctx, type::arr(type::arr(alpha, beta), type::arr(alpha, gamma))));
    BOOST_TEST(conclusion.has_value());
    if (conclusion)
        BOOST_TEST(conclusion->stm.ty == t); // wrong
}

BOOST_AUTO_TEST_CASE(exercise_2_13_b)
{
    context const ctx{{{pre_typed_term::var_t("x"), type::arr(alpha, type::arr(beta, type::arr(alpha, gamma)))}}};
    auto const conclusion = conclusion_of(term_search(ctx, type::arr(alpha, type::arr(type::arr(alpha, beta), gamma))));
    BOOST_TEST(conclusion.has_value());
    if (conclusion)
        BOOST_TEST(conclusion->stm.ty == t); // wrong
}

BOOST_AUTO_TEST_CASE(exercise_2_13_c)
{
    auto const alpha_to_gamma = type::arr(alpha, gamma);
    auto const beta_to_alpha = type::arr(beta, alpha);
    context const ctx{{{pre_typed_term::var_t("x"), type::arr(type::arr(beta, gamma), gamma)}}};
    auto const conclusion = conclusion_of(term_search(ctx, type::arr(alpha_to_gamma, type::arr(beta_to_alpha, gamma))));
    BOOST_TEST(conclusion.has_value());
    if (conclusion)
        BOOST_TEST(conclusion->stm.ty == t); // wrong
    BOOST_TEST(conclusion == judgement(empty, statement(pre_typed_term::var("x"), t)));
}

BOOST_AUTO_TEST_SUITE_END()
