#define BOOST_TEST_MODULE pityp_term
#include <boost/test/included/unit_test.hpp>

#include "libtt/pityp/pre_typed_term.hpp"
#include "libtt/pityp/alpha_equivalence.hpp"
#include "libtt/pityp/pretty_print.hpp"

namespace libtt::pityp {

std::ostream& operator<<(std::ostream& os, type const& t) { return pretty_print(os, t); };
std::ostream& operator<<(std::ostream& os, pre_typed_term const& x) { return pretty_print(os, x); }

}

using namespace libtt::pityp;

boost::test_tools::predicate_result alpha_equivalent(pre_typed_term const& x, pre_typed_term const& y)
{
    if (is_alpha_equivalent(x, y))
        return true;
    boost::test_tools::predicate_result res(false);
    res.message() << "Not alpha equivalent [" << x << " != " << y;
    return res;
}

boost::test_tools::predicate_result not_alpha_equivalent(pre_typed_term const& x, pre_typed_term const& y)
{
    if (not is_alpha_equivalent(x, y))
        return true;
    boost::test_tools::predicate_result res(false);
    res.message() << "Terms are alpha equivalent [" << x << " == " << y;
    return res;
}

BOOST_AUTO_TEST_SUITE(pityp_term_tests)

BOOST_AUTO_TEST_CASE(var_tests)
{
    auto const x = pre_typed_term::var("x");
    auto const y = pre_typed_term::var("y");

    BOOST_TEST(x == x);
    BOOST_TEST(x == pre_typed_term::var("x"));
    BOOST_TEST(y == y);
    BOOST_TEST(y == pre_typed_term::var("y"));
    BOOST_TEST(x != y);
    BOOST_TEST(y != x);
    BOOST_REQUIRE(is_var(x));
    BOOST_REQUIRE(is_var(y));
    BOOST_TEST(not is_app1(x));
    BOOST_TEST(not is_app2(x));
    BOOST_TEST(not is_abs1(x));
    BOOST_TEST(not is_abs2(x));
    BOOST_TEST(std::get<pre_typed_term::var_t>(x.value).name == "x");
    BOOST_TEST(std::get<pre_typed_term::var_t>(y.value).name == "y");
}

BOOST_AUTO_TEST_CASE(app1_tests)
{
    auto const x = pre_typed_term::var("x");
    auto const y = pre_typed_term::var("y");
    auto const xy = pre_typed_term::app(x, y);
    auto const yx = pre_typed_term::app(y, x);

    BOOST_TEST(xy == xy);
    BOOST_TEST(xy == pre_typed_term::app(x, y));
    BOOST_TEST(yx == pre_typed_term::app(y, x));
    BOOST_TEST(xy != yx);
    BOOST_TEST(yx != xy);
    BOOST_TEST(not is_var(xy));
    BOOST_REQUIRE(is_app1(xy));
    BOOST_TEST(not is_app2(xy));
    BOOST_TEST(not is_abs1(xy));
    BOOST_TEST(not is_abs2(xy));
    BOOST_TEST(std::get<pre_typed_term::app1_t>(xy.value).left.get() == pre_typed_term::var("x"));
    BOOST_TEST(std::get<pre_typed_term::app1_t>(xy.value).right.get() == pre_typed_term::var("y"));
}

BOOST_AUTO_TEST_CASE(app2_tests)
{
    auto const s = type::var("s");
    auto const t = type::var("t");
    auto const p = type::pi("s", type::arr(s, t));
    auto const x = pre_typed_term::var("x");
    auto const y = pre_typed_term::var("y");
    auto const xs = pre_typed_term::app(x, s);
    auto const yp = pre_typed_term::app(y, p);

    BOOST_TEST(xs == xs);
    BOOST_TEST(xs == pre_typed_term::app(x, s));
    BOOST_TEST(yp == pre_typed_term::app(y, p));
    BOOST_TEST(xs != yp);
    BOOST_TEST(yp != xs);
    BOOST_TEST(not is_var(xs));
    BOOST_TEST(not is_app1(xs));
    BOOST_REQUIRE(is_app2(xs));
    BOOST_TEST(not is_abs1(xs));
    BOOST_TEST(not is_abs2(xs));
    BOOST_TEST(std::get<pre_typed_term::app2_t>(xs.value).left.get() == pre_typed_term::var("x"));
    BOOST_TEST(std::get<pre_typed_term::app2_t>(xs.value).right == type::var("s"));
}

BOOST_AUTO_TEST_CASE(abs1_tests)
{
    auto const x = pre_typed_term::var("x");
    auto const y = pre_typed_term::var("y");
    auto const var_x = pre_typed_term::var_t("x");
    auto const var_y = pre_typed_term::var_t("y");
    auto const xy = pre_typed_term::app(x, y);
    auto const yx = pre_typed_term::app(y, x);
    auto const fx = pre_typed_term::abs(var_x, type::var("s"), xy);
    auto const fy = pre_typed_term::abs(var_y, type::var("t"), xy);

    BOOST_TEST(fx == fx);
    BOOST_TEST(fx == pre_typed_term::abs(var_x, type::var("s"), xy));
    BOOST_TEST(fy == fy);
    BOOST_TEST(fy == pre_typed_term::abs(var_y, type::var("t"), xy));
    BOOST_TEST(not is_var(fx));
    BOOST_TEST(not is_app1(fx));
    BOOST_TEST(not is_app2(fx));
    BOOST_REQUIRE(is_abs1(fx));
    BOOST_TEST(not is_abs2(fx));
    BOOST_TEST(std::get<pre_typed_term::abs1_t>(fx.value).var.name == "x");
    BOOST_TEST(std::get<pre_typed_term::abs1_t>(fx.value).var_type == type::var("s"));
    BOOST_TEST(std::get<pre_typed_term::abs1_t>(fx.value).body.get() == xy);
}

BOOST_AUTO_TEST_CASE(abs2_tests)
{
    auto const x = pre_typed_term::var("x");
    auto const y = pre_typed_term::var("y");
    auto const var_x = pre_typed_term::var_t("x");
    auto const var_y = pre_typed_term::var_t("y");
    auto const xy = pre_typed_term::app(x, y);
    auto const yx = pre_typed_term::app(y, x);
    auto const fx = pre_typed_term::abs(type::var_t("s"), xy);
    auto const fy = pre_typed_term::abs(type::var_t("t"), xy);

    BOOST_TEST(fx == fx);
    BOOST_TEST(fx == pre_typed_term::abs(type::var_t("s"), xy));
    BOOST_TEST(fy == fy);
    BOOST_TEST(fy == pre_typed_term::abs(type::var_t("t"), xy));
    BOOST_TEST(not is_var(fx));
    BOOST_TEST(not is_app1(fx));
    BOOST_TEST(not is_app2(fx));
    BOOST_TEST(not is_abs1(fx));
    BOOST_REQUIRE(is_abs2(fx));
    BOOST_TEST(std::get<pre_typed_term::abs2_t>(fx.value).var.name == "s");
    BOOST_TEST(std::get<pre_typed_term::abs2_t>(fx.value).body.get() == xy);
}

BOOST_AUTO_TEST_CASE(alpha_equivalence_tests)
{
    auto const x = pre_typed_term::var("x");
    auto const y = pre_typed_term::var("y");
    auto const fx = pre_typed_term::app(pre_typed_term::var("f"), x);
    auto const fy = pre_typed_term::app(pre_typed_term::var("f"), y);
    auto const fs = pre_typed_term::app(pre_typed_term::var("f"), type::var("s"));
    auto const ft = pre_typed_term::app(pre_typed_term::var("f"), type::var("t"));
    auto const lambda_x = pre_typed_term::abs(pre_typed_term::var_t("x"), type::var("s"), x);
    auto const lambda_y = pre_typed_term::abs(pre_typed_term::var_t("y"), type::var("s"), y);
    auto const lambda_x2 = pre_typed_term::abs(pre_typed_term::var_t("x"), type::var("t"), x);
    auto const pi_s = pre_typed_term::abs(type::var_t("s"), lambda_x);
    auto const pi_s2 = pre_typed_term::abs(type::var_t("s"), lambda_x2);
    auto const pi_t = pre_typed_term::abs(type::var_t("t"), lambda_x2);

    BOOST_TEST(alpha_equivalent(x, x));
    BOOST_TEST(not_alpha_equivalent(x, y));
    BOOST_TEST(not_alpha_equivalent(x, fx));
    BOOST_TEST(not_alpha_equivalent(x, fy));
    BOOST_TEST(not_alpha_equivalent(x, fs));
    BOOST_TEST(not_alpha_equivalent(x, ft));
    BOOST_TEST(not_alpha_equivalent(x, lambda_x));
    BOOST_TEST(not_alpha_equivalent(x, lambda_y));
    BOOST_TEST(not_alpha_equivalent(x, lambda_x2));
    BOOST_TEST(not_alpha_equivalent(x, pi_s));
    BOOST_TEST(not_alpha_equivalent(x, pi_s2));
    BOOST_TEST(not_alpha_equivalent(x, pi_t));

    BOOST_TEST(not_alpha_equivalent(y, x));
    BOOST_TEST(alpha_equivalent(y, y));
    BOOST_TEST(not_alpha_equivalent(y, fx));
    BOOST_TEST(not_alpha_equivalent(y, fy));
    BOOST_TEST(not_alpha_equivalent(y, fs));
    BOOST_TEST(not_alpha_equivalent(y, ft));
    BOOST_TEST(not_alpha_equivalent(y, lambda_x));
    BOOST_TEST(not_alpha_equivalent(y, lambda_y));
    BOOST_TEST(not_alpha_equivalent(y, lambda_x2));
    BOOST_TEST(not_alpha_equivalent(y, pi_s));
    BOOST_TEST(not_alpha_equivalent(y, pi_s2));
    BOOST_TEST(not_alpha_equivalent(y, pi_t));

    BOOST_TEST(not_alpha_equivalent(fx, x));
    BOOST_TEST(not_alpha_equivalent(fx, y));
    BOOST_TEST(alpha_equivalent(fx, fx));
    BOOST_TEST(not_alpha_equivalent(fx, fy));
    BOOST_TEST(not_alpha_equivalent(fx, fs));
    BOOST_TEST(not_alpha_equivalent(fx, ft));
    BOOST_TEST(not_alpha_equivalent(fx, lambda_x));
    BOOST_TEST(not_alpha_equivalent(fx, lambda_y));
    BOOST_TEST(not_alpha_equivalent(fx, lambda_x2));
    BOOST_TEST(not_alpha_equivalent(fx, pi_s));
    BOOST_TEST(not_alpha_equivalent(fx, pi_s2));
    BOOST_TEST(not_alpha_equivalent(fx, pi_t));

    BOOST_TEST(not_alpha_equivalent(fy, x));
    BOOST_TEST(not_alpha_equivalent(fy, y));
    BOOST_TEST(not_alpha_equivalent(fy, fx));
    BOOST_TEST(alpha_equivalent(fy, fy));
    BOOST_TEST(not_alpha_equivalent(fy, fs));
    BOOST_TEST(not_alpha_equivalent(fy, ft));
    BOOST_TEST(not_alpha_equivalent(fy, lambda_x));
    BOOST_TEST(not_alpha_equivalent(fy, lambda_y));
    BOOST_TEST(not_alpha_equivalent(fy, lambda_x2));
    BOOST_TEST(not_alpha_equivalent(fy, pi_s));
    BOOST_TEST(not_alpha_equivalent(fy, pi_s2));
    BOOST_TEST(not_alpha_equivalent(fy, pi_t));

    BOOST_TEST(not_alpha_equivalent(fs, x));
    BOOST_TEST(not_alpha_equivalent(fs, y));
    BOOST_TEST(not_alpha_equivalent(fs, fx));
    BOOST_TEST(not_alpha_equivalent(fs, fy));
    BOOST_TEST(alpha_equivalent(fs, fs));
    BOOST_TEST(not_alpha_equivalent(fs, ft));
    BOOST_TEST(not_alpha_equivalent(fs, lambda_x));
    BOOST_TEST(not_alpha_equivalent(fs, lambda_y));
    BOOST_TEST(not_alpha_equivalent(fs, lambda_x2));
    BOOST_TEST(not_alpha_equivalent(fs, pi_s));
    BOOST_TEST(not_alpha_equivalent(fs, pi_s2));
    BOOST_TEST(not_alpha_equivalent(fs, pi_t));

    BOOST_TEST(not_alpha_equivalent(ft, x));
    BOOST_TEST(not_alpha_equivalent(ft, y));
    BOOST_TEST(not_alpha_equivalent(ft, fx));
    BOOST_TEST(not_alpha_equivalent(ft, fy));
    BOOST_TEST(not_alpha_equivalent(ft, fs));
    BOOST_TEST(alpha_equivalent(ft, ft));
    BOOST_TEST(not_alpha_equivalent(ft, lambda_x));
    BOOST_TEST(not_alpha_equivalent(ft, lambda_y));
    BOOST_TEST(not_alpha_equivalent(ft, lambda_x2));
    BOOST_TEST(not_alpha_equivalent(ft, pi_s));
    BOOST_TEST(not_alpha_equivalent(ft, pi_s2));
    BOOST_TEST(not_alpha_equivalent(ft, pi_t));

    BOOST_TEST(not_alpha_equivalent(lambda_x, x));
    BOOST_TEST(not_alpha_equivalent(lambda_x, y));
    BOOST_TEST(not_alpha_equivalent(lambda_x, fx));
    BOOST_TEST(not_alpha_equivalent(lambda_x, fy));
    BOOST_TEST(not_alpha_equivalent(lambda_x, fs));
    BOOST_TEST(not_alpha_equivalent(lambda_x, ft));
    BOOST_TEST(alpha_equivalent(lambda_x, lambda_x));
    BOOST_TEST(alpha_equivalent(lambda_x, lambda_y));
    BOOST_TEST(not_alpha_equivalent(lambda_x, lambda_x2));
    BOOST_TEST(not_alpha_equivalent(lambda_x, pi_s));
    BOOST_TEST(not_alpha_equivalent(lambda_x, pi_s2));
    BOOST_TEST(not_alpha_equivalent(lambda_x, pi_t));

    BOOST_TEST(not_alpha_equivalent(lambda_y, x));
    BOOST_TEST(not_alpha_equivalent(lambda_y, y));
    BOOST_TEST(not_alpha_equivalent(lambda_y, fx));
    BOOST_TEST(not_alpha_equivalent(lambda_y, fy));
    BOOST_TEST(not_alpha_equivalent(lambda_y, fs));
    BOOST_TEST(not_alpha_equivalent(lambda_y, ft));
    BOOST_TEST(alpha_equivalent(lambda_y, lambda_x));
    BOOST_TEST(alpha_equivalent(lambda_y, lambda_y));
    BOOST_TEST(not_alpha_equivalent(lambda_y, lambda_x2));
    BOOST_TEST(not_alpha_equivalent(lambda_y, pi_s));
    BOOST_TEST(not_alpha_equivalent(lambda_y, pi_s2));
    BOOST_TEST(not_alpha_equivalent(lambda_y, pi_t));

    BOOST_TEST(not_alpha_equivalent(lambda_x2, x));
    BOOST_TEST(not_alpha_equivalent(lambda_x2, y));
    BOOST_TEST(not_alpha_equivalent(lambda_x2, fx));
    BOOST_TEST(not_alpha_equivalent(lambda_x2, fy));
    BOOST_TEST(not_alpha_equivalent(lambda_x2, fs));
    BOOST_TEST(not_alpha_equivalent(lambda_x2, ft));
    BOOST_TEST(not_alpha_equivalent(lambda_x2, lambda_x));
    BOOST_TEST(not_alpha_equivalent(lambda_x2, lambda_y));
    BOOST_TEST(alpha_equivalent(lambda_x2, lambda_x2));
    BOOST_TEST(not_alpha_equivalent(lambda_x2, pi_s));
    BOOST_TEST(not_alpha_equivalent(lambda_x2, pi_s2));
    BOOST_TEST(not_alpha_equivalent(lambda_x2, pi_t));

    BOOST_TEST(not_alpha_equivalent(pi_s, x));
    BOOST_TEST(not_alpha_equivalent(pi_s, y));
    BOOST_TEST(not_alpha_equivalent(pi_s, fx));
    BOOST_TEST(not_alpha_equivalent(pi_s, fy));
    BOOST_TEST(not_alpha_equivalent(pi_s, fs));
    BOOST_TEST(not_alpha_equivalent(pi_s, ft));
    BOOST_TEST(not_alpha_equivalent(pi_s, lambda_x));
    BOOST_TEST(not_alpha_equivalent(pi_s, lambda_y));
    BOOST_TEST(not_alpha_equivalent(pi_s, lambda_x2));
    BOOST_TEST(alpha_equivalent(pi_s, pi_s));
    BOOST_TEST(not_alpha_equivalent(pi_s, pi_s2));
    BOOST_TEST(alpha_equivalent(pi_s, pi_t));

    BOOST_TEST(not_alpha_equivalent(pi_s2, x));
    BOOST_TEST(not_alpha_equivalent(pi_s2, y));
    BOOST_TEST(not_alpha_equivalent(pi_s2, fx));
    BOOST_TEST(not_alpha_equivalent(pi_s2, fy));
    BOOST_TEST(not_alpha_equivalent(pi_s2, fs));
    BOOST_TEST(not_alpha_equivalent(pi_s2, ft));
    BOOST_TEST(not_alpha_equivalent(pi_s2, lambda_x));
    BOOST_TEST(not_alpha_equivalent(pi_s2, lambda_y));
    BOOST_TEST(not_alpha_equivalent(pi_s2, lambda_x2));
    BOOST_TEST(not_alpha_equivalent(pi_s2, pi_s));
    BOOST_TEST(alpha_equivalent(pi_s2, pi_s2));
    BOOST_TEST(not_alpha_equivalent(pi_s2, pi_t));

    BOOST_TEST(not_alpha_equivalent(pi_t, x));
    BOOST_TEST(not_alpha_equivalent(pi_t, y));
    BOOST_TEST(not_alpha_equivalent(pi_t, fx));
    BOOST_TEST(not_alpha_equivalent(pi_t, fy));
    BOOST_TEST(not_alpha_equivalent(pi_t, fs));
    BOOST_TEST(not_alpha_equivalent(pi_t, ft));
    BOOST_TEST(not_alpha_equivalent(pi_t, lambda_x));
    BOOST_TEST(not_alpha_equivalent(pi_t, lambda_y));
    BOOST_TEST(not_alpha_equivalent(pi_t, lambda_x2));
    BOOST_TEST(alpha_equivalent(pi_t, pi_s));
    BOOST_TEST(not_alpha_equivalent(pi_t, pi_s2));
    BOOST_TEST(alpha_equivalent(pi_t, pi_t));
}

BOOST_AUTO_TEST_SUITE_END()
