#define BOOST_TEST_MODULE pitype_legal_term
#include <boost/test/included/unit_test.hpp>

#include "libtt/pityp/legal_term.hpp"
#include "libtt/pityp/pretty_print.hpp"

namespace libtt::pityp {

template <typename T>
std::ostream& operator<<(std::ostream& os, std::optional<T> const& x)
{
    return x ? os << *x : os << "nullopt";
}

std::ostream& operator<<(std::ostream& os, context::type_decl_t const& x) { return pretty_print(os, x); }
std::ostream& operator<<(std::ostream& os, context::var_decl_t const& x) { return pretty_print(os, x); }
std::ostream& operator<<(std::ostream& os, legal_term const& x) { return pretty_print(os, x); }
std::ostream& operator<<(std::ostream& os, type const& x) { return pretty_print(os, x); }
std::ostream& operator<<(std::ostream& os, pre_typed_term const& x) { return pretty_print(os, x); }

template <typename T1, typename T2, typename... Ts>
pre_typed_term app(T1 const& x1, T2 const& x2, Ts const&... xs)
{
    if constexpr (sizeof...(Ts) == 0)
        return pre_typed_term::app(x1, x2);
    else
        return app(pre_typed_term::app(x1, x2), xs...);
}

}

using namespace libtt::pityp;

BOOST_AUTO_TEST_SUITE(pitype_legal_term_tests)

BOOST_AUTO_TEST_CASE(constructor_tests)
{
    auto const var_s = type::var_t("s");
    auto const var_x = pre_typed_term::var_t("x");
    auto const s = type::var("s");
    auto const x = pre_typed_term::var("x");
    context ctx;
    ctx = extend(ctx, var_s).value();
    ctx = extend(ctx, var_x, s).value();

    auto const mx = legal_term(type_assign(ctx, x).value());
    BOOST_TEST(std::size(decls_of(mx.ctx())) == 2ul);
    BOOST_TEST(mx.ctx()[var_s] == context::type_decl_t(var_s));
    BOOST_TEST(mx.ctx()[var_x] == context::var_decl_t(var_x, s));
    BOOST_TEST(mx.term() == x);
    BOOST_TEST(mx.ty() == s);
}

BOOST_AUTO_TEST_SUITE_END()
