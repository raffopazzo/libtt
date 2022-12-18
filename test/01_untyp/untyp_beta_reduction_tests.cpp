#define BOOST_TEST_MODULE untyp_beta_reduction
#include <boost/test/included/unit_test.hpp>

#include "libtt/untyp/beta_reduction.hpp"

template <typename R, typename T>
bool contains(R const& r, T const& x)
{
    return std::ranges::find(r, x) != std::ranges::end(r);
}

namespace std
{
    template <typename T>
    std::ostream& operator<<(std::ostream& os, std::optional<T> const& x)
    {
        if (x)
            return os << *x;
        else
            return os << "nullopt";
    }

    std::ostream& operator<<(std::ostream& os, std::nullopt_t)
    {
        return os << "nullopt";
    }
}

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

BOOST_AUTO_TEST_SUITE(untyp_beta_reduction_tests)

BOOST_AUTO_TEST_CASE(is_redex_tests)
{
    using namespace libtt::untyp;
    auto const v = term::var("v");
    auto const x = term::var("x");
    auto const y = term::var("y");
    auto const z = term::var("z");

    BOOST_TEST(not is_redex(x));
    BOOST_TEST(not is_redex(term::app(x, x)));
    BOOST_TEST(not is_redex(term::abs("x", x)));

    auto const abs1 = term::abs("x", term::app(term::abs("y", term::app(y, x)), z));
    auto const app1 = term::app(abs1, v);
    BOOST_TEST(is_redex(app1));

    auto const abs2 = term::abs("x", term::app(x, x));
    auto const omega = term::app(abs2, abs2);
    BOOST_TEST(is_redex(omega));

    auto const app2_1 = term::app(x, omega);
    auto const app2_2 = term::app(omega, x);
    BOOST_TEST(not is_redex(app2_1));
    BOOST_TEST(not is_redex(app2_2));

    auto const const_v = term::abs("u", v);
    auto const app3_1 = term::app(const_v, omega);
    auto const app3_2 = term::app(omega, const_v);
    BOOST_TEST(is_redex(app3_1));
    BOOST_TEST(not is_redex(app3_2));

    auto const abs3 = term::abs("x", omega);
    BOOST_TEST(not is_redex(abs3));

    auto const delta = term::abs("x", make_app({x, x, x}));
    auto const delta2 = term::app(delta, delta);
    auto const delta3 = term::app(delta2, delta);
    BOOST_TEST(is_redex(delta2));
    BOOST_TEST(not is_redex(delta3));
}

BOOST_AUTO_TEST_CASE(num_redexes_tests)
{
    using namespace libtt::untyp;
    auto const v = term::var("v");
    auto const x = term::var("x");
    auto const y = term::var("y");
    auto const z = term::var("z");

    BOOST_TEST(num_redexes(x) == 0ul);
    BOOST_TEST(num_redexes(term::app(x, x)) == 0ul);
    BOOST_TEST(num_redexes(term::abs("x", x)) == 0ul);

    auto const abs1 = term::abs("x", term::app(term::abs("y", term::app(y, x)), z));
    auto const app1 = term::app(abs1, v);
    BOOST_TEST(num_redexes(app1) == 2ul);

    auto const abs2 = term::abs("x", term::app(x, x));
    auto const omega = term::app(abs2, abs2);
    BOOST_TEST(num_redexes(omega) == 1ul);

    auto const app2_1 = term::app(x, omega);
    auto const app2_2 = term::app(omega, x);
    BOOST_TEST(num_redexes(app2_1) == 1ul);
    BOOST_TEST(num_redexes(app2_2) == 1ul);

    auto const const_v = term::abs("u", v);
    auto const app3_1 = term::app(const_v, omega);
    auto const app3_2 = term::app(omega, const_v);
    BOOST_TEST(num_redexes(app3_1) == 2ul);

    auto const abs3 = term::abs("x", omega);
    BOOST_TEST(num_redexes(abs3) == 1ul);

    auto const delta = term::abs("x", make_app({x, x, x}));
    auto const delta2 = term::app(delta, delta);
    auto const delta3 = term::app(delta2, delta);
    BOOST_TEST(num_redexes(delta2) == 1ul);
}

BOOST_AUTO_TEST_CASE(one_step_beta_reductions_tests)
{
    using namespace libtt::untyp;
    auto const v = term::var("v");
    auto const x = term::var("x");
    auto const y = term::var("y");
    auto const z = term::var("z");

    BOOST_TEST(one_step_beta_reductions(x).empty());
    BOOST_TEST(one_step_beta_reductions(term::app(x, x)).empty());
    BOOST_TEST(one_step_beta_reductions(term::abs("x", x)).empty());

    auto const abs1 = term::abs("x", term::app(term::abs("y", term::app(y, x)), z));
    auto const app1 = term::app(abs1, v);
    BOOST_TEST(one_step_beta_reductions(app1).size() == 2);
    BOOST_TEST(contains(one_step_beta_reductions(app1), term::app(term::abs("y", term::app(y, v)), z)));
    BOOST_TEST(contains(one_step_beta_reductions(app1), term::app(term::abs("x", term::app(z, x)), v)));

    auto const abs2 = term::abs("x", term::app(x, x));
    auto const omega = term::app(abs2, abs2);
    BOOST_TEST(one_step_beta_reductions(omega) == std::vector{omega}, boost::test_tools::per_element{});

    auto const app2_1 = term::app(x, omega);
    auto const app2_2 = term::app(omega, x);
    BOOST_TEST(one_step_beta_reductions(app2_1) == std::vector{app2_1}, boost::test_tools::per_element{});
    BOOST_TEST(one_step_beta_reductions(app2_2) == std::vector{app2_2}, boost::test_tools::per_element{});

    auto const const_v = term::abs("u", v);
    auto const app3_1 = term::app(const_v, omega);
    auto const app3_2 = term::app(omega, const_v);
    BOOST_TEST(one_step_beta_reductions(app3_1).size() == 2);
    BOOST_TEST(contains(one_step_beta_reductions(app3_1), v));
    BOOST_TEST(contains(one_step_beta_reductions(app3_1), app3_1));

    auto const abs3 = term::abs("x", omega);
    BOOST_TEST(one_step_beta_reductions(abs3) == std::vector{abs3}, boost::test_tools::per_element{});

    auto const delta = term::abs("x", make_app({x, x, x}));
    auto const delta2 = term::app(delta, delta);
    auto const delta3 = term::app(delta2, delta);
    BOOST_TEST(one_step_beta_reductions(delta2) == std::vector{delta3}, boost::test_tools::per_element{});
}

BOOST_AUTO_TEST_CASE(beta_normalize_tests)
{
    using namespace libtt::untyp;
    auto const v = term::var("v");
    auto const x = term::var("x");
    auto const y = term::var("y");
    auto const z = term::var("z");
    auto const xx = term::app(x, x);

    BOOST_TEST(beta_normalize(x) == x);
    BOOST_TEST(beta_normalize(xx) == xx);
    BOOST_TEST(beta_normalize(term::abs("x", x)) == term::abs("x", x));

    auto const abs1 = term::abs("x", term::app(term::abs("y", term::app(y, x)), z));
    auto const app1 = term::app(abs1, v);
    BOOST_TEST(beta_normalize(app1) == term::app(z, v));

    auto const omega = term::app(term::abs("x", xx), term::abs("x", xx));
    BOOST_TEST(beta_normalize(omega) == std::nullopt);

    auto const const_v = term::abs("u", v);
    auto const app2_1 = term::app(const_v, omega);
    auto const app2_2 = term::app(omega, const_v);
    BOOST_TEST(beta_normalize(app2_1) == v);
    BOOST_TEST(beta_normalize(app2_2) == std::nullopt);

    auto const delta = term::abs("x", make_app({x, x, x}));
    BOOST_TEST(beta_normalize(term::app(delta, delta)) == std::nullopt);

    auto const app3 = term::app(term::abs("x", xx), term::app(term::abs("x", xx), const_v));
    BOOST_TEST(beta_normalize(app3) == term::app(v, v));

    auto const x4_combinator = term::abs("x", term::app(xx, xx));
    BOOST_TEST(beta_normalize(term::app(x4_combinator, const_v)) == term::app(v, v));
}

BOOST_AUTO_TEST_SUITE_END()
