#define BOOST_TEST_MODULE pre_typed_term
#include <boost/test/included/unit_test.hpp>

#include "libtt/typed/pre_typed_term.hpp"
#include "libtt/typed/pretty_print.hpp"

namespace libtt::typed {

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
}

using namespace libtt::typed;

BOOST_AUTO_TEST_SUITE(pre_typed_term_tests)

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
    BOOST_TEST(not is_app(x));
    BOOST_TEST(not is_abs(x));
    BOOST_TEST(std::get<pre_typed_term::var_t>(x.value).name == "x");
    BOOST_TEST(std::get<pre_typed_term::var_t>(y.value).name == "y");
}

BOOST_AUTO_TEST_CASE(app_tests)
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
    BOOST_REQUIRE(is_app(xy));
    BOOST_TEST(not is_abs(xy));
    BOOST_TEST(std::get<pre_typed_term::app_t>(xy.value).left.get() == pre_typed_term::var("x"));
    BOOST_TEST(std::get<pre_typed_term::app_t>(xy.value).right.get() == pre_typed_term::var("y"));
}

BOOST_AUTO_TEST_CASE(abs_tests)
{
    auto const x = pre_typed_term::var("x");
    auto const y = pre_typed_term::var("y");
    auto const xy = pre_typed_term::app(x, y);
    auto const yx = pre_typed_term::app(y, x);
    auto const fx = pre_typed_term::abs("x", type::var("s"), xy);
    auto const fy = pre_typed_term::abs("y", type::var("t"), xy);

    BOOST_TEST(fx == fx);
    BOOST_TEST(fx == pre_typed_term::abs("x", type::var("s"), xy));
    BOOST_TEST(fy == fy);
    BOOST_TEST(fy == pre_typed_term::abs("y", type::var("t"), xy));
    BOOST_TEST(not is_var(fx));
    BOOST_TEST(not is_app(fx));
    BOOST_REQUIRE(is_abs(fx));
    BOOST_TEST(std::get<pre_typed_term::abs_t>(fx.value).var == pre_typed_term::var_t("x"));
    BOOST_TEST(std::get<pre_typed_term::abs_t>(fx.value).var_type == type::var("s"));
    BOOST_TEST(std::get<pre_typed_term::abs_t>(fx.value).body.get() == xy);
}

BOOST_AUTO_TEST_SUITE_END()
