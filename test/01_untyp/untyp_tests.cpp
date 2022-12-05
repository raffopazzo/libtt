#define BOOST_TEST_MODULE untyp
#include <boost/test/included/unit_test.hpp>

#include "libtt/untyp.hpp"

struct term_ostream_visitor
{
    std::ostream& os;

    std::ostream& operator()(libtt::untyp::term::var_t const&);
    std::ostream& operator()(libtt::untyp::term::app_t const&);
    std::ostream& operator()(libtt::untyp::term::abs_t const&);
};

namespace libtt::untyp
{
    std::ostream& operator<<(std::ostream& os, libtt::untyp::term const& x)
    {
	return std::visit(term_ostream_visitor{os}, x.value);
    }
    std::ostream& operator<<(std::ostream& os, libtt::untyp::term::var_t const& x)
    {
        return os << x.name;
    }
}

std::ostream& term_ostream_visitor::operator()(libtt::untyp::term::var_t const& x)
{
    return os << x.name;
}
std::ostream& term_ostream_visitor::operator()(libtt::untyp::term::app_t const& x)
{
    return os << '(' << x.left.get() << ' ' << x.right.get() << ')';
}

std::ostream& term_ostream_visitor::operator()(libtt::untyp::term::abs_t const& x)
{
    return os << "(lambda " << x.var.name << " . "<< x.body.get() << ')';
}

BOOST_AUTO_TEST_SUITE(untyp_tests)

BOOST_AUTO_TEST_CASE(variable_tests)
{
    auto const x = libtt::untyp::term::var("x");
    auto const y = libtt::untyp::term::var("y");
    BOOST_TEST(is_var(x));
    BOOST_TEST(not is_app(x));
    BOOST_TEST(not is_abs(x));
    BOOST_TEST(x != y);
    BOOST_TEST(x == libtt::untyp::term::var("x"));
}

BOOST_AUTO_TEST_CASE(application_tests)
{
    auto const x = libtt::untyp::term::var("x");
    auto const y = libtt::untyp::term::var("y");
    auto const app1 = libtt::untyp::term::app(x, y);
    auto const app2 = libtt::untyp::term::app(x, x);
    BOOST_TEST(not is_var(app1));
    BOOST_TEST(is_app(app1));
    BOOST_TEST(not is_abs(app1));
    BOOST_TEST(app1 != app2);
    BOOST_TEST(app1 == libtt::untyp::term::app(x, y));
    BOOST_TEST(app2 == libtt::untyp::term::app(x, x));
}

BOOST_AUTO_TEST_CASE(abstraction_tests)
{
    auto const x = libtt::untyp::term::var("x");
    auto const y = libtt::untyp::term::var("y");
    auto const app1 = libtt::untyp::term::app(x, y);
    auto const app2 = libtt::untyp::term::app(x, x);
    auto const abs1 = libtt::untyp::term::abs("x", app1);
    auto const abs2 = libtt::untyp::term::abs("y", app1);
    BOOST_TEST(not is_var(abs1));
    BOOST_TEST(not is_app(abs1));
    BOOST_TEST(is_abs(abs1));
    BOOST_TEST(abs1 != abs2);
    BOOST_TEST(abs1 == libtt::untyp::term::abs("x", app1));
    BOOST_TEST(abs2 == libtt::untyp::term::abs("y", app1));
}

BOOST_AUTO_TEST_CASE(subterms_tests)
{
    auto const x = libtt::untyp::term::var("x");
    auto const y = libtt::untyp::term::var("y");
    auto const app1 = libtt::untyp::term::app(x, y);
    auto const app2 = libtt::untyp::term::app(x, x);
    auto const abs1 = libtt::untyp::term::abs("x", app1);
    auto const abs2 = libtt::untyp::term::abs("y", app1);
    auto const abs3 = libtt::untyp::term::abs("x", app2);
    auto const app3 = libtt::untyp::term::app(abs3, abs3);
    BOOST_TEST(libtt::untyp::subterms(x) == std::vector{x});
    BOOST_TEST(libtt::untyp::subterms(y) == std::vector{y});
    auto expected = std::vector{app1, x, y};
    BOOST_TEST(libtt::untyp::subterms(app1) == expected, boost::test_tools::per_element{});
    expected = std::vector{app2, x, x};
    BOOST_TEST(libtt::untyp::subterms(app2) == expected, boost::test_tools::per_element{});
    expected = std::vector{abs1, app1, x, y};
    BOOST_TEST(libtt::untyp::subterms(abs1) == expected, boost::test_tools::per_element{});
    expected = std::vector{abs2, app1, x, y};
    BOOST_TEST(libtt::untyp::subterms(abs2) == expected, boost::test_tools::per_element{});
    expected = std::vector{abs3, app2, x, x};
    BOOST_TEST(libtt::untyp::subterms(abs3) == expected, boost::test_tools::per_element{});
    expected = std::vector{app3, abs3, app2, x, x, abs3, app2, x, x};
    BOOST_TEST(libtt::untyp::subterms(app3) == expected, boost::test_tools::per_element{});
}

BOOST_AUTO_TEST_CASE(proper_subterms_tests)
{
    auto const x = libtt::untyp::term::var("x");
    auto const y = libtt::untyp::term::var("y");
    auto const app1 = libtt::untyp::term::app(x, y);
    auto const app2 = libtt::untyp::term::app(x, x);
    auto const abs1 = libtt::untyp::term::abs("x", app1);
    auto const abs2 = libtt::untyp::term::abs("y", app1);
    auto const abs3 = libtt::untyp::term::abs("x", app2);
    auto const app3 = libtt::untyp::term::app(abs3, abs3);
    BOOST_TEST(libtt::untyp::proper_subterms(x).empty());
    BOOST_TEST(libtt::untyp::proper_subterms(y).empty());
    auto expected = std::vector{x, y};
    BOOST_TEST(libtt::untyp::proper_subterms(app1) == expected, boost::test_tools::per_element{});
    expected = std::vector{x, x};
    BOOST_TEST(libtt::untyp::proper_subterms(app2) == expected, boost::test_tools::per_element{});
    expected = std::vector{app1, x, y};
    BOOST_TEST(libtt::untyp::proper_subterms(abs1) == expected, boost::test_tools::per_element{});
    expected = std::vector{app1, x, y};
    BOOST_TEST(libtt::untyp::proper_subterms(abs2) == expected, boost::test_tools::per_element{});
    expected = std::vector{app2, x, x};
    BOOST_TEST(libtt::untyp::proper_subterms(abs3) == expected, boost::test_tools::per_element{});
    expected = std::vector{abs3, app2, x, x, abs3, app2, x, x};
    BOOST_TEST(libtt::untyp::proper_subterms(app3) == expected, boost::test_tools::per_element{});
}

BOOST_AUTO_TEST_CASE(pretty_printer_tests)
{
    auto const x = libtt::untyp::term::var("x");
    auto const y = libtt::untyp::term::var("y");
    auto const app1 = libtt::untyp::term::app(x, y);
    auto const app2 = libtt::untyp::term::app(x, x);
    auto const abs1 = libtt::untyp::term::abs("x", app1);
    auto const abs2 = libtt::untyp::term::abs("y", app1);
    auto const abs3 = libtt::untyp::term::abs("x", app2);
    auto const app3 = libtt::untyp::term::app(abs3, abs3);
    auto const abs4 = libtt::untyp::term::abs("x", abs2);
    auto const app4 = libtt::untyp::term::app(app2, app2);
    auto const abs5 = libtt::untyp::term::abs("z", abs4);
    auto const app5 = libtt::untyp::term::app(x, app2);
    auto const app6 = libtt::untyp::term::app(abs1, y);
    auto const app7 = libtt::untyp::term::app(x, abs1);
    auto const pretty_print = [] (libtt::untyp::term const& x)
    {
	std::stringstream ss;
	libtt::untyp::pretty_print(ss, x);
	return ss.str();
    };
    BOOST_TEST(pretty_print(x) == "x");
    BOOST_TEST(pretty_print(y) == "y");
    BOOST_TEST(pretty_print(app1) == "x y");
    BOOST_TEST(pretty_print(app2) == "x x");
    BOOST_TEST(pretty_print(abs1) == "lambda x . x y");
    BOOST_TEST(pretty_print(abs2) == "lambda y . x y");
    BOOST_TEST(pretty_print(abs3) == "lambda x . x x");
    BOOST_TEST(pretty_print(app3) == "(lambda x . x x)(lambda x . x x)");
    BOOST_TEST(pretty_print(abs4) == "lambda x y . x y");
    BOOST_TEST(pretty_print(app4) == "(x x)(x x)");
    BOOST_TEST(pretty_print(abs5) == "lambda z x y . x y");
    BOOST_TEST(pretty_print(app5) == "x(x x)");
    BOOST_TEST(pretty_print(app6) == "(lambda x . x y)y");
    BOOST_TEST(pretty_print(app7) == "x(lambda x . x y)");
}

BOOST_AUTO_TEST_CASE(free_variables_tests)
{
    using var_t = libtt::untyp::term::var_t;
    auto const x = libtt::untyp::term::var("x");
    auto const y = libtt::untyp::term::var("y");
    auto const app1 = libtt::untyp::term::app(x, y);
    auto const app2 = libtt::untyp::term::app(x, x);
    auto const abs1 = libtt::untyp::term::abs("x", app1);
    auto const abs2 = libtt::untyp::term::abs("y", app1);
    auto const abs3 = libtt::untyp::term::abs("x", app2);
    auto const app3 = libtt::untyp::term::app(abs3, abs3);
    auto const abs4 = libtt::untyp::term::abs("x", abs2);
    auto const app4 = libtt::untyp::term::app(app2, app2);
    auto const abs5 = libtt::untyp::term::abs("z", abs4);
    auto const app5 = libtt::untyp::term::app(x, app2);
    auto const app6 = libtt::untyp::term::app(abs1, y);
    auto const app7 = libtt::untyp::term::app(x, abs1);
    auto expected = std::set{var_t{"x"}};
    BOOST_TEST(libtt::untyp::free_variables(x) == expected, boost::test_tools::per_element{});
    expected = std::set{var_t{"y"}};
    BOOST_TEST(libtt::untyp::free_variables(y) == expected, boost::test_tools::per_element{});
    expected = std::set{var_t{"x"}, var_t{"y"}};
    BOOST_TEST(libtt::untyp::free_variables(app1) == expected, boost::test_tools::per_element{});
    expected = std::set{var_t{"x"}};
    BOOST_TEST(libtt::untyp::free_variables(app2) == expected, boost::test_tools::per_element{});
    expected = std::set{var_t{"y"}};
    BOOST_TEST(libtt::untyp::free_variables(abs1) == expected, boost::test_tools::per_element{});
    expected = std::set{var_t{"x"}};
    BOOST_TEST(libtt::untyp::free_variables(abs2) == expected, boost::test_tools::per_element{});
    expected = {};
    BOOST_TEST(libtt::untyp::free_variables(abs3) == expected, boost::test_tools::per_element{});
    BOOST_TEST(libtt::untyp::free_variables(app3) == expected, boost::test_tools::per_element{});
    BOOST_TEST(libtt::untyp::free_variables(abs4) == expected, boost::test_tools::per_element{});
    expected = std::set{var_t{"x"}};
    BOOST_TEST(libtt::untyp::free_variables(app4) == expected, boost::test_tools::per_element{});
    expected = {};
    BOOST_TEST(libtt::untyp::free_variables(abs5) == expected, boost::test_tools::per_element{});
    expected = std::set{var_t{"x"}};
    BOOST_TEST(libtt::untyp::free_variables(app5) == expected, boost::test_tools::per_element{});
    expected = std::set{var_t{"y"}};
    BOOST_TEST(libtt::untyp::free_variables(app6) == expected, boost::test_tools::per_element{});
    expected = std::set{var_t{"x"}, var_t{"y"}};
    BOOST_TEST(libtt::untyp::free_variables(app7) == expected, boost::test_tools::per_element{});
}

BOOST_AUTO_TEST_CASE(closed_term_tests)
{
    using namespace libtt::untyp;
    auto const x = term::var("x");
    auto const y = term::var("y");
    auto const xxy = term::app(term::app(x, x), y);
    BOOST_TEST(not is_closed(term::abs("x", xxy)));
    BOOST_TEST(is_closed(term::abs("x", term::abs("y", xxy))));
    BOOST_TEST(is_closed(term::abs("x", term::abs("y", term::abs("z", xxy)))));
}

BOOST_AUTO_TEST_SUITE_END()
