#define BOOST_TEST_MODULE term_search
#include <boost/test/included/unit_test.hpp>

#include "libtt/typed/alpha_equivalence.hpp"
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

std::ostream& operator<<(std::ostream& os, type const& x) { return pretty_print(os, x); };
std::ostream& operator<<(std::ostream& os, pre_typed_term::var_t const& x) { return os << x.name; };
std::ostream& operator<<(std::ostream& os, pre_typed_term const& x) { return pretty_print(os, x); };
std::ostream& operator<<(std::ostream& os, statement const& x) { return pretty_print(os, x); }
std::ostream& operator<<(std::ostream& os, judgement const& x) { return pretty_print(os, x); }

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
    BOOST_TEST(conclusion_of(term_search(ctx, s_to_t)) == judgement(ctx, statement(pre_typed_term::var("f"), s_to_t)));
}

BOOST_AUTO_TEST_CASE(apply_function_from_context)
{
    context const ctx{{
        {pre_typed_term::var_t("f"), s_to_t},
        {pre_typed_term::var_t("u"), s}
    }};
    auto const expected_stm = statement(pre_typed_term::app(pre_typed_term::var("f"), pre_typed_term::var("u")), t);
    BOOST_TEST(conclusion_of(term_search(ctx, t)) == judgement(ctx, expected_stm));
}

BOOST_AUTO_TEST_CASE(apply_multi_variate)
{
    context const ctx{{
        {pre_typed_term::var_t("f"), type::arr(alpha, s_to_t)},
        {pre_typed_term::var_t("x"), alpha},
        {pre_typed_term::var_t("u"), s}
    }};
    auto const expected_stm = statement(
        pre_typed_term::app(
            pre_typed_term::app(
                pre_typed_term::var("f"),
                pre_typed_term::var("x")),
            pre_typed_term::var("u")),
        t);
    BOOST_TEST(conclusion_of(term_search(ctx, t)) == judgement(ctx, expected_stm));
}

BOOST_AUTO_TEST_CASE(exercise_2_13_a)
{
    context const ctx{{{pre_typed_term::var_t("x"), type::arr(alpha, type::arr(beta, gamma))}}};
    auto const expected_type = type::arr(type::arr(alpha, beta), type::arr(alpha, gamma));
    auto const conclusion = conclusion_of(term_search(ctx, expected_type));
    BOOST_TEST(conclusion.has_value());
    if (conclusion)
    {
        auto const equivalent_term =
            pre_typed_term::abs(
                "u", type::arr(alpha, beta),
                pre_typed_term::abs(
                    "v", alpha,
                    pre_typed_term::app(
                        pre_typed_term::app(
                            pre_typed_term::var("x"),
                            pre_typed_term::var("v")),
                        pre_typed_term::app(
                            pre_typed_term::var("u"),
                            pre_typed_term::var("v")))));
        BOOST_TEST(conclusion->ctx.decls == ctx.decls, boost::test_tools::per_element{});
        BOOST_TEST(conclusion->stm.ty == expected_type);
        BOOST_TEST(is_alpha_equivalent(conclusion->stm.subject, equivalent_term));
    }
}

BOOST_AUTO_TEST_CASE(exercise_2_13_b)
{
    context const ctx{{{pre_typed_term::var_t("x"), type::arr(alpha, type::arr(beta, type::arr(alpha, gamma)))}}};
    auto const expected_type = type::arr(alpha, type::arr(type::arr(alpha, beta), gamma));
    auto const conclusion = conclusion_of(term_search(ctx, expected_type));
    BOOST_TEST(conclusion.has_value());
    if (conclusion)
    {
        auto const equivalent_term =
            pre_typed_term::abs(
                "u", alpha,
                pre_typed_term::abs(
                    "v", type::arr(alpha, beta),
                    pre_typed_term::app(
                        pre_typed_term::app(
                            pre_typed_term::app(
                                pre_typed_term::var("x"),
                                pre_typed_term::var("u")),
                            pre_typed_term::app(
                                pre_typed_term::var("v"),
                                pre_typed_term::var("u"))),
                        pre_typed_term::var("u"))));
        BOOST_TEST(conclusion->ctx.decls == ctx.decls, boost::test_tools::per_element{});
        BOOST_TEST(conclusion->stm.ty == expected_type);
        BOOST_TEST(is_alpha_equivalent(conclusion->stm.subject, equivalent_term));
    }
}

BOOST_AUTO_TEST_CASE(exercise_2_13_c)
{
    auto const alpha_to_gamma = type::arr(alpha, gamma);
    auto const beta_to_alpha = type::arr(beta, alpha);
    context const ctx{{{pre_typed_term::var_t("x"), type::arr(type::arr(beta, gamma), gamma)}}};
    auto const expected_type = type::arr(alpha_to_gamma, type::arr(beta_to_alpha, gamma));
    auto const conclusion = conclusion_of(term_search(ctx, expected_type));
    BOOST_TEST(conclusion.has_value());
    if (conclusion)
    {
        auto const equivalent_term =
            pre_typed_term::abs(
                "u", alpha_to_gamma,
                pre_typed_term::abs(
                    "v", beta_to_alpha,
                    pre_typed_term::app(
                        pre_typed_term::var("x"),
                        pre_typed_term::abs(
                            "w", beta,
                            pre_typed_term::app(
                                pre_typed_term::var("u"),
                                pre_typed_term::app(
                                    pre_typed_term::var("v"),
                                    pre_typed_term::var("w")))))));
        BOOST_TEST(conclusion->ctx.decls == ctx.decls, boost::test_tools::per_element{});
        BOOST_TEST(conclusion->stm.ty == expected_type);
        BOOST_TEST(is_alpha_equivalent(conclusion->stm.subject, equivalent_term));
    }
}

BOOST_AUTO_TEST_CASE(avoid_infinite_recursion)
{
    context const ctx{{{pre_typed_term::var_t("f"), type::arr(s, s)}}};
    BOOST_TEST(not conclusion_of(term_search(ctx, s)).has_value());
}

BOOST_AUTO_TEST_SUITE_END()
