#define BOOST_TEST_MODULE untyp_alpha_equivalence
#include <boost/test/included/unit_test.hpp>

#include "libtt/untyp/alpha_equivalence.hpp"
#include "libtt/untyp/pretty_print.hpp"

namespace libtt::untyp
{
    std::ostream& operator<<(std::ostream& os, term const& x)
    {
        return pretty_print(os, x);
    }
}

using namespace libtt::untyp;

static term make_abs(std::vector<std::string> vars, term body)
{
    auto it = vars.rbegin();
    auto rv = term::abs(*it++, body);
    while (it != vars.rend())
        rv = term::abs(*it++, rv);
    return rv;
}

static term make_app(std::vector<term> xs)
{
    auto it = xs.begin();
    auto rv = *it++;
    while (it != xs.end())
        rv = term::app(rv, *it++);
    return rv;
}

BOOST_AUTO_TEST_SUITE(untyp_alpha_equivalence_tests)

BOOST_AUTO_TEST_CASE(alpha_equivalence_tests)
{
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

BOOST_AUTO_TEST_CASE(lemma_1_6_5_test)
{
    // Lemma 1.6.5 page 13 of Nederpelt-Geuvers
    // Let x != y and assume x not a free variable in L. Then:
    // M[x:=N][y:=L] == M[y:=L][x:=N[y:=L]]
    auto const x = term::var("x");
    auto const y = term::var("y");
    auto const z = term::var("z");
    auto const M = term::abs("y", term::app(y, x));
    auto const N = term::abs("x", term::app(y, x));
    auto const L = make_abs({"x", "y"}, make_app({z, z, x}));
    auto const M_x_N = substitute(M, term::var_t("x"), N);
    auto const M_y_L = substitute(M, term::var_t("y"), L);
    auto const N_y_L = substitute(N, term::var_t("y"), L);
    auto const lhs = substitute(M_x_N, term::var_t("y"), L);
    auto const rhs = substitute(M_y_L, term::var_t("x"), N_y_L);
    BOOST_TEST(is_alpha_equivalent(lhs, rhs));
}

BOOST_AUTO_TEST_SUITE_END()
