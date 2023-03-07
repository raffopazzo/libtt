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

//template <typename T, typename U>
//std::ostream& operator<<(std::ostream& os, std::pair<T, U> const& x)
//{
//    return os << x.first << ':' << x.second;
//}
//
//std::ostream& operator<<(std::ostream& os, type const& x) { return pretty_print(os, x); }
//std::ostream& operator<<(std::ostream& os, pre_typed_term const& x) { return pretty_print(os, x); }
//std::ostream& operator<<(std::ostream& os, statement const& x) { return pretty_print(os, x); }
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

//BOOST_AUTO_TEST_CASE(exercise_2_9)
//{
//    auto const a = type::var("a");
//    auto const b = type::var("b");
//    auto const c = type::var("c");
//    auto const d = type::var("d");
//    auto const ctx = context{{
//        {pre_typed_term::var_t("x"), type::arr(d, type::arr(d, a))},
//        {pre_typed_term::var_t("y"), type::arr(c, a)},
//        {pre_typed_term::var_t("z"), type::arr(a, b)}
//    }};
//    auto const d_c_b = type::arr(d, type::arr(c, b));
//    auto const u = pre_typed_term::var("u");
//    auto const v = pre_typed_term::var("v");
//    auto const x = pre_typed_term::var("x");
//    auto const y = pre_typed_term::var("y");
//    auto const z = pre_typed_term::var("z");
//    auto const abs = [] (auto... args) { return pre_typed_term::abs(args...); };
//    auto const app = [] (auto... args) { return pre_typed_term::app(args...); };
//    auto const expr_a = abs("u", d, abs("v", c, app(z, app(y, v))));
//    auto const expr_b = abs("u", d, abs( "v", c, app(z, app(app(x, u), u))));
//    BOOST_TEST(conclusion_of(type_assign(ctx, expr_a)) == judgement(ctx, statement(expr_a, d_c_b)));
//    BOOST_TEST(conclusion_of(type_assign(ctx, expr_b)) == judgement(ctx, statement(expr_b, d_c_b)));
//}

BOOST_AUTO_TEST_SUITE_END()
