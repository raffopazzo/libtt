#define BOOST_TEST_MODULE legal_term
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

BOOST_AUTO_TEST_CASE(constructor_tests)
{
    auto const s = type::var("s");
    auto const x = pre_typed_term::var("x");
    context const ctx{{{pre_typed_term::var_t("x"), s}}};

    auto const mx = legal_term(type_assign(ctx, x).value());
    BOOST_TEST(mx.ctx().decls == ctx.decls, boost::test_tools::per_element{});
    BOOST_TEST(mx.term() == x);
    BOOST_TEST(mx.ty() == s);
}

BOOST_AUTO_TEST_SUITE_END()
