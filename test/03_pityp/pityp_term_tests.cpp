#define BOOST_TEST_MODULE pityp_term
#include <boost/test/included/unit_test.hpp>

#include "libtt/pityp/pre_typed_term.hpp"
#include "libtt/pityp/pretty_print.hpp"

namespace libtt::pityp {

std::ostream& operator<<(std::ostream& os, type const& t) { return pretty_print(os, t); };
std::ostream& operator<<(std::ostream& os, pre_typed_term const& x) { return pretty_print(os, x); }

}

using namespace libtt::pityp;

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

BOOST_AUTO_TEST_SUITE_END()
