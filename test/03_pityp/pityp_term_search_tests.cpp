#define BOOST_TEST_MODULE pityp_term_search
#include <boost/test/included/unit_test.hpp>

#include "libtt/pityp/derivation.hpp"
#include "libtt/pityp/alpha_equivalence.hpp"
#include "libtt/pityp/pretty_print.hpp"

namespace libtt::pityp {

template <typename T>
std::ostream& operator<<(std::ostream& os, std::optional<T> const& x)
{
    return x ? (os << *x) : (os << "nullopt");
}

std::ostream& operator<<(std::ostream& os, term_judgement_t const& x) { return pretty_print(os, x); }

}

using namespace libtt::pityp;

boost::test_tools::predicate_result alpha_equivalent(std::optional<term_judgement_t> const& a, term_judgement_t b)
{
    // TODO: shouldn't `a->stm.ty == b.stm.ty` be replaced with `is_alpha_equivalent(a->stm.ty, b.stm.ty)` ?
    if (a and a->ctx == b.ctx and a->stm.ty == b.stm.ty and is_alpha_equivalent(a->stm.subject, b.stm.subject))
        return true;
    boost::test_tools::predicate_result res(false);
    res.message() << "Not alpha equivalent [" << a << " != " << b;
    return res;
}

struct pityp_term_search_tests_fixture
{
    type const s = type::var("s");
    type const t = type::var("t");
    type const s_to_s = type::arr(s, s);
    type const s_to_t = type::arr(s, t);
    pre_typed_term const x = pre_typed_term::var("x");
    pre_typed_term const f = pre_typed_term::var("f");
};

BOOST_FIXTURE_TEST_SUITE(pityp_term_search_tests, pityp_term_search_tests_fixture)

BOOST_AUTO_TEST_CASE(should_find_term_using_var_rule)
{
    context ctx;
    ctx = extend(ctx, type::var_t("s")).value();
    ctx = extend(ctx, type::var_t("t")).value();
    ctx = extend(ctx, pre_typed_term::var_t("x"), s).value();
    ctx = extend(ctx, pre_typed_term::var_t("f"), s_to_t).value();
    BOOST_TEST(conclusion_of(term_search(ctx, s)) == term_judgement_t(ctx, term_stm_t(x, s)));
    BOOST_TEST(conclusion_of(term_search(ctx, s_to_t)) == term_judgement_t(ctx, term_stm_t(f, s_to_t)));
}

BOOST_AUTO_TEST_CASE(apply_function_from_context)
{
    auto const ctx1 = [&]
    {
        context ctx;
        ctx = extend(ctx, type::var_t("s")).value();
        ctx = extend(ctx, type::var_t("t")).value();
        ctx = extend(ctx, pre_typed_term::var_t("f"), type::arr(s, t)).value();
        ctx = extend(ctx, pre_typed_term::var_t("x"), s).value();
        return ctx;
    }();
    auto const ctx2 = [&]
    {
        context ctx;
        ctx = extend(ctx, type::var_t("s")).value();
        ctx = extend(ctx, type::var_t("t")).value();
        ctx = extend(ctx, pre_typed_term::var_t("f"), type::arr(s, s_to_t)).value();
        ctx = extend(ctx, pre_typed_term::var_t("x"), s).value();
        return ctx;
    }();
    BOOST_TEST(
        conclusion_of(term_search(ctx1, t)) ==
        term_judgement_t(ctx1, term_stm_t(pre_typed_term::app(f, x), t)));
    BOOST_TEST(
        conclusion_of(term_search(ctx2, s_to_t)) ==
        term_judgement_t(ctx2, term_stm_t(pre_typed_term::app(f, x), s_to_t)));
}

BOOST_AUTO_TEST_CASE(apply_multivariate_function)
{
    context ctx;
    ctx = extend(ctx, type::var_t("s")).value();
    ctx = extend(ctx, type::var_t("t")).value();
    ctx = extend(ctx, pre_typed_term::var_t("f"), type::arr(s, s_to_t)).value();
    ctx = extend(ctx, pre_typed_term::var_t("x"), s).value();
    BOOST_TEST(
        conclusion_of(term_search(ctx, t)) ==
        term_judgement_t(ctx, term_stm_t(pre_typed_term::app(pre_typed_term::app(f, x), x), t)));
}

BOOST_AUTO_TEST_CASE(partial_application_of_multivariate_function)
{
    context ctx;
    ctx = extend(ctx, type::var_t("s")).value();
    ctx = extend(ctx, type::var_t("t")).value();
    ctx = extend(ctx, pre_typed_term::var_t("f"), type::arr(s, type::arr(s, s_to_t))).value();
    ctx = extend(ctx, pre_typed_term::var_t("x"), s).value();
    BOOST_TEST(
        conclusion_of(term_search(ctx, s_to_t)) ==
        term_judgement_t(ctx, term_stm_t(pre_typed_term::app(pre_typed_term::app(f, x), x), s_to_t)));
}

BOOST_AUTO_TEST_CASE(should_find_term_of_first_order_abstraction)
{
    auto const ctx1 = [&]
    {
        context ctx;
        ctx = extend(ctx, type::var_t("s")).value();
        ctx = extend(ctx, type::var_t("t")).value();
        ctx = extend(ctx, pre_typed_term::var_t("x"), t).value();
        return ctx;
    }();
    BOOST_TEST(alpha_equivalent(
        conclusion_of(term_search(ctx1, s_to_t)),
        term_judgement_t(ctx1, term_stm_t(pre_typed_term::abs(pre_typed_term::var_t("y"), s, x), s_to_t))));
    // the type of the abstracted variable must be legal
    auto const ctx2 = [&]
    {
        context ctx;
        ctx = extend(ctx, type::var_t("t")).value();
        ctx = extend(ctx, pre_typed_term::var_t("x"), t).value();
        return ctx;
    }();
    BOOST_TEST(not conclusion_of(term_search(ctx2, s_to_t)).has_value());
}

BOOST_AUTO_TEST_CASE(simple_second_order_application)
{
    auto const ctx1 = [&]
    {
        context ctx;
        ctx = extend(ctx, type::var_t("s")).value();
        ctx = extend(ctx, pre_typed_term::var_t("f"), type::pi("a", type::arr(type::var("a"), type::var("a")))).value();
        return ctx;
    }();
    // note that `lambda x:s . x` is a valid result, but probably less useful than `f s` so we prefer the latter
    BOOST_TEST(
        conclusion_of(term_search(ctx1, s_to_s)) ==
        term_judgement_t(ctx1, term_stm_t(pre_typed_term::app(f, s), s_to_s)));

    auto const ctx2 = [&]
    {
        context ctx;
        ctx = extend(ctx, type::var_t("s")).value();
        ctx = extend(ctx, pre_typed_term::var_t("f"), type::pi("a", type::var("a"))).value();
        return ctx;
    }();
    // note that `lambda x:s . x` is a valid result, but probably less useful than `f (s->s)` so we prefer the latter
    BOOST_TEST(
        conclusion_of(term_search(ctx2, s_to_s)) ==
        term_judgement_t(ctx2, term_stm_t(pre_typed_term::app(f, s_to_s), s_to_s)));
}

BOOST_AUTO_TEST_CASE(recursive_second_order_application)
{
    context ctx;
    ctx = extend(ctx, type::var_t("s")).value();
    ctx = extend(ctx, type::var_t("t")).value();
    ctx = extend(ctx, pre_typed_term::var_t("f"), type::pi("a", type::pi("b", type::arr(type::var("a"), type::var("b"))))).value();
    BOOST_TEST(
        conclusion_of(term_search(ctx, s_to_s)) ==
        term_judgement_t(ctx, term_stm_t(pre_typed_term::app(pre_typed_term::app(f, s), s), s_to_s)));
    BOOST_TEST(
        conclusion_of(term_search(ctx, s_to_t)) ==
        term_judgement_t(ctx, term_stm_t(pre_typed_term::app(pre_typed_term::app(f, s), t), s_to_t)));
}

BOOST_AUTO_TEST_CASE(absurd_test)
{
    auto const absurdity = type::pi("a", type::var("a"));
    context const ctx1 = [&]
    {
        context ctx;
        ctx = extend(ctx, type::var_t("s")).value();
        ctx = extend(ctx, pre_typed_term::var_t("x"), absurdity).value();
        return ctx;
    }();
    BOOST_TEST(conclusion_of(term_search(ctx1, s)) == term_judgement_t(ctx1, term_stm_t(pre_typed_term::app(x, s), s)));

    context const ctx2 = [&]
    {
        context ctx;
        ctx = extend(ctx, type::var_t("s")).value();
        ctx = extend(ctx, type::var_t("t")).value();
        ctx = extend(ctx, pre_typed_term::var_t("x"), type::arr(s, absurdity)).value();
        ctx = extend(ctx, pre_typed_term::var_t("f"), type::arr(type::arr(s, s), s)).value();
        return ctx;
    }();
    auto const alpha = pre_typed_term::app(f, pre_typed_term::abs(pre_typed_term::var_t("x"), s, x));
    auto const beta = pre_typed_term::app(pre_typed_term::app(x, alpha), t);
    BOOST_TEST(alpha_equivalent(conclusion_of(term_search(ctx2, s)), term_judgement_t(ctx2, term_stm_t(alpha, s))));
    BOOST_TEST(alpha_equivalent(conclusion_of(term_search(ctx2, t)), term_judgement_t(ctx2, term_stm_t(beta, t))));

    context const ctx3 = [&]
    {
        context ctx;
        ctx = extend(ctx, type::var_t("s")).value();
        ctx = extend(ctx, type::var_t("t")).value();
        ctx = extend(ctx, pre_typed_term::var_t("x"), s).value();
        ctx = extend(ctx, pre_typed_term::var_t("f"), type::arr(s, type::arr(s, absurdity))).value();
        return ctx;
    }();
    BOOST_TEST(
        conclusion_of(term_search(ctx3, t)) ==
        term_judgement_t(ctx3, term_stm_t(pre_typed_term::app(pre_typed_term::app(pre_typed_term::app(f, x), x), t), t)));
}

BOOST_AUTO_TEST_SUITE_END()
