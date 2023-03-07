#define BOOST_TEST_MODULE pityp_derivation
#include <boost/test/included/unit_test.hpp>

#include "libtt/pityp/derivation.hpp"
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

BOOST_AUTO_TEST_SUITE(pityp_derivation_tests)

BOOST_AUTO_TEST_CASE(var_test)
{
    auto const s = type::var("s");
    auto const x = pre_typed_term::var("x");

    context const empty;
    context const ctx = [&]
    {
        context ctx;
        ctx = extend(ctx, type::var_t("s")).value();
        ctx = extend(ctx, pre_typed_term::var_t("x"), s).value();
        return ctx;
    }();
    BOOST_TEST(not conclusion_of(type_assign(empty, x)).has_value());
    BOOST_TEST(conclusion_of(type_assign(ctx, x)) == term_judgement_t(ctx, term_stm_t(x, s)));
}

BOOST_AUTO_TEST_CASE(app1_test)
{
    auto const s = type::var("s");
    auto const t = type::var("t");
    auto const fx = pre_typed_term::app(pre_typed_term::var("f"), pre_typed_term::var("x"));

    context const empty;
    context const ctx = [&]
    {
        context ctx;
        ctx = extend(ctx, type::var_t("s")).value();
        ctx = extend(ctx, type::var_t("t")).value();
        ctx = extend(ctx, pre_typed_term::var_t("f"), type::arr(s, t)).value();
        ctx = extend(ctx, pre_typed_term::var_t("x"), s).value();
        return ctx;
    }();
    BOOST_TEST(not conclusion_of(type_assign(empty, fx)).has_value());
    BOOST_TEST(conclusion_of(type_assign(ctx, fx)) == term_judgement_t(ctx, term_stm_t(fx, t)));
}

BOOST_AUTO_TEST_CASE(app2_test)
{
    auto const p = type::pi("a", type::arr(type::var("a"), type::var("a")));
    auto const s = type::var("s");
    auto const f = pre_typed_term::var("f");
    auto const fs = pre_typed_term::app(f, s);

    context const empty;
    context const ctx = [&]
    {
        context ctx;
        ctx = extend(ctx, pre_typed_term::var_t("f"), p).value();
        ctx = extend(ctx, type::var_t("s")).value();
        return ctx;
    }();
    BOOST_TEST(not conclusion_of(type_assign(empty, fs)).has_value());
    BOOST_TEST(conclusion_of(type_assign(ctx, fs)) == term_judgement_t(ctx, term_stm_t(fs, type::arr(s, s))));
}

BOOST_AUTO_TEST_CASE(abs1_test)
{
    auto const s = type::var("s");
    auto const t = type::var("t");
    auto const s_to_t = type::arr(s, t);
    auto const g = pre_typed_term::var("g");
    auto const x = pre_typed_term::var("x");
    auto const f = pre_typed_term::abs(pre_typed_term::var_t("g"), s_to_t, pre_typed_term::app(g, x));

    context const empty;
    context const ctx = [&]
    {
        context ctx;
        ctx = extend(ctx, type::var_t("s")).value();
        ctx = extend(ctx, type::var_t("t")).value();
        ctx = extend(ctx, pre_typed_term::var_t("x"), s).value();
        return ctx;
    }();
    BOOST_TEST(not conclusion_of(type_assign(empty, f)).has_value());
    BOOST_TEST(conclusion_of(type_assign(ctx, f)) == term_judgement_t(ctx, term_stm_t(f, type::arr(s_to_t, t))));
}

BOOST_AUTO_TEST_CASE(abs2_test)
{
    auto const s = type::var("s");
    auto const p = type::pi("s", type::arr(s, s));
    auto const f =
        pre_typed_term::abs(
            type::var_t("s"),
            pre_typed_term::abs(pre_typed_term::var_t("x"), s, pre_typed_term::var("x")));
    context const empty;
    BOOST_TEST(conclusion_of(type_assign(empty, f)) == term_judgement_t(empty, term_stm_t(f, p)));
}

BOOST_AUTO_TEST_SUITE_END()
