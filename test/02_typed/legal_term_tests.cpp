#define BOOST_TEST_MODULE pre_typed_term
#include <boost/test/included/unit_test.hpp>

#include "libtt/typed/legal_term.hpp"
#include "libtt/typed/pretty_print.hpp"

namespace libtt::typed {

template <typename T, typename U>
std::ostream& operator<<(std::ostream& os, std::pair<T, U> const& x)
{
    return os << x.first << ':' << x.second;
}

std::ostream& operator<<(std::ostream& os, type const& x) { return pretty_print(os, x); }
std::ostream& operator<<(std::ostream& os, pre_typed_term::var_t const& x) { return os << x.name; }
std::ostream& operator<<(std::ostream& os, pre_typed_term const& x) { return pretty_print(os, x); }

}

using namespace libtt::typed;

BOOST_AUTO_TEST_SUITE(legal_term_tests)

BOOST_AUTO_TEST_CASE(substitution_tests)
{
    auto const s = type::var("s");
    auto const t = type::var("t");
    auto const s_to_t = type::arr(s, t);
    auto const x = pre_typed_term::var("x");
    auto const y = pre_typed_term::var("y");
    context const ctx1{{{pre_typed_term::var_t("x"), s}}};
    context const ctx2{{{pre_typed_term::var_t("y"), t}}};

    auto const m = legal_term(type_assign(ctx1, x).value());
    BOOST_TEST(m.ctx.decls == ctx1.decls, boost::test_tools::per_element{});
    BOOST_TEST(m.term == x);
    BOOST_TEST(m.ty == s);
}

BOOST_AUTO_TEST_SUITE_END()
