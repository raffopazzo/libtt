#define BOOST_TEST_MODULE untyp
#include <boost/test/included/unit_test.hpp>

#include "libtt/untyp.hpp"

namespace libtt::untyp
{
    std::ostream& operator<<(std::ostream&, term const&);
    std::ostream& operator<<(std::ostream&, term::var_t const&);
    std::ostream& operator<<(std::ostream&, term::app_t const&);
    std::ostream& operator<<(std::ostream&, term::abs_t const&);

    std::ostream& operator<<(std::ostream& os, term const& x)
    {
	return std::visit([&os] (auto const& y) -> std::ostream& { return os << y; }, x.value);
    }
    std::ostream& operator<<(std::ostream& os, term::var_t const& x)
    {
        return os << x.name;
    }
    std::ostream& operator<<(std::ostream& os, term::app_t const& x)
    {
        return os << '(' << x.left.get() << ' ' << x.right.get() << ')';
    }
    std::ostream& operator<<(std::ostream& os, term::abs_t const& x)
    {
        return os << "(lambda " << x.var << " . "<< x.body.get() << ')';
    }

    static term make_app(std::vector<term> xs)
    {
        auto it = xs.begin();
        auto rv = *it++;
        while (it != xs.end())
            rv = term::app(rv, *it++);
        return rv;
    }

    static term make_abs(std::vector<std::string> vars, term body)
    {
        auto it = vars.rbegin();
        auto rv = term::abs(*it++, body);
        while (it != vars.rend())
            rv = term::abs(*it++, rv);
        return rv;
    }
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

BOOST_AUTO_TEST_CASE(binding_variables_tests)
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
    auto expected = std::set<var_t>{};
    BOOST_TEST(libtt::untyp::binding_variables(x) == expected, boost::test_tools::per_element{});
    BOOST_TEST(libtt::untyp::binding_variables(y) == expected, boost::test_tools::per_element{});
    BOOST_TEST(libtt::untyp::binding_variables(app1) == expected, boost::test_tools::per_element{});
    BOOST_TEST(libtt::untyp::binding_variables(app2) == expected, boost::test_tools::per_element{});
    expected = std::set{var_t{"x"}};
    BOOST_TEST(libtt::untyp::binding_variables(abs1) == expected, boost::test_tools::per_element{});
    expected = std::set{var_t{"y"}};
    BOOST_TEST(libtt::untyp::binding_variables(abs2) == expected, boost::test_tools::per_element{});
    expected = std::set{var_t{"x"}};
    BOOST_TEST(libtt::untyp::binding_variables(abs3) == expected, boost::test_tools::per_element{});
    BOOST_TEST(libtt::untyp::binding_variables(app3) == expected, boost::test_tools::per_element{});
    expected = std::set{var_t{"x"}, var_t{"y"}};
    BOOST_TEST(libtt::untyp::binding_variables(abs4) == expected, boost::test_tools::per_element{});
    expected = {};
    BOOST_TEST(libtt::untyp::binding_variables(app4) == expected, boost::test_tools::per_element{});
    expected = std::set{var_t{"x"}, var_t{"y"}, var_t{"z"}};
    BOOST_TEST(libtt::untyp::binding_variables(abs5) == expected, boost::test_tools::per_element{});
    expected = {};
    BOOST_TEST(libtt::untyp::binding_variables(app5) == expected, boost::test_tools::per_element{});
    expected = std::set{var_t{"x"}};
    BOOST_TEST(libtt::untyp::binding_variables(app6) == expected, boost::test_tools::per_element{});
    BOOST_TEST(libtt::untyp::binding_variables(app7) == expected, boost::test_tools::per_element{});
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

BOOST_AUTO_TEST_CASE(replace_tests)
{
    using namespace libtt::untyp;
    auto const x = term::var("x");
    auto const y = term::var("y");
    auto const var_x = term::var_t{"x"};
    auto const var_y = term::var_t{"y"};
    auto const app1 = term::app(x, y);
    auto const abs1 = term::abs(var_x, x);
    auto const abs2 = term::abs(var_x, term::app(x, y));
    BOOST_TEST(replace(app1, var_x, var_y) == term::app(y, y));
    BOOST_TEST(replace(abs1, var_x, var_y) == abs1);
    BOOST_TEST(replace(abs2, var_y, var_x) == term::abs(var_x, term::app(x, x)));
}

BOOST_AUTO_TEST_CASE(rename_tests)
{
    using namespace libtt::untyp;
    auto const x = term::var("x");
    auto const y = term::var("y");
    auto const var_x = term::var_t{"x"};

    // basic renaming case
    auto const abs1 = term::abs_t(var_x, x);
    auto const renamed1 = rename(abs1);
    BOOST_TEST(renamed1.var != abs1.var);
    BOOST_TEST(renamed1 == term::abs_t(renamed1.var, term::var(renamed1.var.name)));
    BOOST_TEST(renamed1 == rename(abs1)); // rename must be deterministic

    // allow the user to specify which variable names cannot be used for renaming
    auto const renamed1_2 = rename(abs1, {renamed1.var});
    BOOST_TEST(renamed1_2.var != renamed1.var);
    BOOST_TEST(renamed1_2 == term::abs_t(renamed1_2.var, term::var(renamed1_2.var.name)));
    BOOST_TEST(renamed1_2 == rename(abs1, {renamed1.var})); // rename must be deterministic

    // rename recursively
    auto const abs2 = term::abs_t(var_x, term::app(x, y));
    auto const renamed2 = rename(abs2);
    BOOST_TEST(renamed2.var != abs2.var);
    BOOST_TEST(renamed2 == term::abs_t(renamed2.var, term::app(term::var(renamed2.var.name), y)));
    BOOST_TEST(renamed2 == rename(abs2));// rename must be deterministic

    // renaming an internal abstraction over the same name should do nothing on the internal abstraction
    auto const abs3 = term::abs_t(var_x, term::abs(var_x, term::app(x, x)));
    auto const renamed3 = rename(abs3);
    BOOST_TEST(renamed3.var != abs3.var);
    BOOST_TEST(renamed3 == term::abs_t(renamed3.var, term::abs(var_x, term::app(x, x))));
    BOOST_TEST(renamed3 == rename(abs3)); // rename must be deterministic

    // cannot use internal binding variables
    auto const abs4 = term::abs_t(var_x, term::abs(renamed1.var, term::var(renamed1.var.name)));
    auto const renamed4 = rename(abs4);
    BOOST_TEST(renamed4.var != abs4.var);
    BOOST_TEST(renamed4.var != renamed1.var);
    BOOST_TEST(renamed4 == term::abs_t(renamed4.var, term::abs(renamed1.var, term::var(renamed1.var.name))));
    BOOST_TEST(renamed4 == rename(abs4)); // rename must be deterministic
}

BOOST_AUTO_TEST_CASE(alpha_equivalence_tests)
{
    using namespace libtt::untyp;
    auto const x = term::var("x");
    auto const y = term::var("y");
    auto const z = term::var("z");
    auto const u = term::var("u");
    auto const v = term::var("v");
    auto const abs1_1 = term::app(term::abs("x", term::app(x, term::abs("z", term::app(x, y)))), z);
    auto const abs1_2 = term::app(term::abs("u", term::app(u, term::abs("z", term::app(u, y)))), z);
    auto const abs1_3 = term::app(term::abs("z", term::app(z, term::abs("x", term::app(z, y)))), z);
    auto const abs1_4 = term::app(term::abs("y", term::app(y, term::abs("z", term::app(y, y)))), z);
    auto const abs1_5 = term::app(term::abs("z", term::app(z, term::abs("z", term::app(z, y)))), z);
    auto const abs1_6 = term::app(term::abs("u", term::app(u, term::abs("z", term::app(u, y)))), v);
    BOOST_TEST(is_alpha_equivalent(abs1_1, abs1_1));
    BOOST_TEST(is_alpha_equivalent(abs1_1, abs1_2));
    BOOST_TEST(is_alpha_equivalent(abs1_1, abs1_3));
    BOOST_TEST(not is_alpha_equivalent(abs1_1, abs1_4));
    BOOST_TEST(not is_alpha_equivalent(abs1_1, abs1_5));
    BOOST_TEST(not is_alpha_equivalent(abs1_1, abs1_6));

    auto const abs2_0 = term::abs("x", term::abs("y", term::app(term::app(x, z), y)));
    auto const abs2_1 = term::abs("v", term::abs("y", term::app(term::app(v, z), y)));
    auto const abs2_2 = term::abs("v", term::abs("u", term::app(term::app(v, z), u)));
    auto const abs2_3 = term::abs("y", term::abs("y", term::app(term::app(y, z), y)));
    auto const abs2_4 = term::abs("z", term::abs("y", term::app(term::app(z, z), y)));
    BOOST_TEST(is_alpha_equivalent(abs2_0, abs2_1));
    BOOST_TEST(is_alpha_equivalent(abs2_0, abs2_2));
    BOOST_TEST(not is_alpha_equivalent(abs2_0, abs2_3));
    BOOST_TEST(not is_alpha_equivalent(abs2_0, abs2_4));
}

BOOST_AUTO_TEST_CASE(substitute_tests)
{
    using namespace libtt::untyp;
    auto const x = term::var("x");
    auto const y = term::var("y");
    auto const z = term::var("z");
    auto const xy = term::app(x, y);

    auto const abs1 = term::abs("y", term::app(y, x));
    auto const sub1 = substitute(abs1, term::var_t("x"), xy);
    BOOST_REQUIRE(std::holds_alternative<term::abs_t>(sub1.value));
    auto const var1 = std::get<term::abs_t>(sub1.value).var;
    BOOST_TEST(var1.name != "y");
    BOOST_TEST(sub1 == term::abs(var1, term::app(term::var(var1.name), xy)));

    auto const abs2 = term::abs("x", term::app(y, x));
    auto const sub2 = substitute(abs2, term::var_t("x"), xy);
    BOOST_TEST(sub2 == abs2); // `x` is a binding variable, so substitution should not take place

    auto const abs3 = make_abs({"x", "y"}, make_app({z, z, x}));
    auto const sub3 = substitute(abs3, term::var_t("z"), y);
    BOOST_TEST(sub3 != abs3);
    BOOST_TEST(is_alpha_equivalent(sub3, make_abs({"x", "v"}, make_app({y, y, x}))));
    BOOST_TEST(not is_alpha_equivalent(sub3, make_abs({"x", "y"}, make_app({y, y, x}))));
}


BOOST_AUTO_TEST_SUITE_END()
