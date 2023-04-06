#define BOOST_TEST_MODULE legal_term_beta_reduction
#include <boost/test/included/unit_test.hpp>

#include "libtt/typed/legal_term.hpp"
#include "libtt/typed/beta_reduction.hpp"
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
std::ostream& operator<<(std::ostream& os, legal_term const& x)
{
    os << '{';
    for (auto const& kv: x.ctx().decls)
        os << kv;
    os << '}';
    return os << " => " << x.term() << " : " << x.ty();
}

}

using namespace libtt::typed;

struct legal_term_beta_reduction_tests_fixture
{
    type const s = type::var("s");
    type const t = type::var("t");
    type const s_to_s = type::arr(s, s);
    context const empty;
    pre_typed_term const x = pre_typed_term::var("x");
    pre_typed_term const y = pre_typed_term::var("y");
    pre_typed_term const id = pre_typed_term::abs("z", s, pre_typed_term::var("z"));
};

BOOST_FIXTURE_TEST_SUITE(legal_term_beta_reduction_tests, legal_term_beta_reduction_tests_fixture)

BOOST_AUTO_TEST_CASE(variable_is_beta_normal)
{
    context const ctx{{{pre_typed_term::var_t("x"), s}}};
    auto const m = legal_term(type_assign(ctx, x).value());
    BOOST_TEST(num_redexes(m) == 0ul);
    BOOST_TEST(not is_redex(m));
    BOOST_TEST(is_beta_normal(m));
    BOOST_TEST(beta_normalize(m) == m);
}

BOOST_AUTO_TEST_CASE(identity_is_beta_normal)
{
    auto const m = legal_term(type_assign(empty, id).value());
    BOOST_TEST(num_redexes(m) == 0ul);
    BOOST_TEST(not is_redex(m));
    BOOST_TEST(is_beta_normal(m));
    BOOST_TEST(beta_normalize(m) == m);
}

BOOST_AUTO_TEST_CASE(identity_reduces_to_argument)
{
    context const ctx{{{pre_typed_term::var_t("x"), s}}};
    auto const m = legal_term(type_assign(ctx, pre_typed_term::app(id, x)).value());
    BOOST_TEST(num_redexes(m) == 1ul);
    BOOST_TEST(is_redex(m));
    BOOST_TEST(not is_beta_normal(m));
    BOOST_TEST(beta_normalize(m) != m);

    auto const n = beta_normalize(m);
    BOOST_TEST(num_redexes(n) == 0ul);
    BOOST_TEST(not is_redex(n));
    BOOST_TEST(is_beta_normal(n));
    BOOST_TEST(beta_normalize(n) == n);
    BOOST_TEST(n.term() == x);
}

BOOST_AUTO_TEST_CASE(applications_with_function_from_context_dont_reduce)
{
    context const ctx{{{pre_typed_term::var_t("x"), s}, {pre_typed_term::var_t("f"), s_to_s}}};
    auto const m = legal_term(type_assign(ctx, pre_typed_term::app(pre_typed_term::var("f"), x)).value());
    BOOST_TEST(num_redexes(m) == 0ul);
    BOOST_TEST(not is_redex(m));
    BOOST_TEST(is_beta_normal(m));
    BOOST_TEST(beta_normalize(m) == m);
}

BOOST_AUTO_TEST_CASE(can_beta_reduce_recursively)
{
    auto const m = legal_term(type_assign(empty, pre_typed_term::abs("x", s, pre_typed_term::app(id, x))).value());
    BOOST_TEST(num_redexes(m) == 1ul);
    BOOST_TEST(not is_redex(m)); // it contains a redex, but it isn't one itself
    BOOST_TEST(not is_beta_normal(m));
    BOOST_TEST(beta_normalize(m) != m);

    auto const n = beta_normalize(m);
    BOOST_TEST(num_redexes(n) == 0ul);
    BOOST_TEST(not is_redex(n));
    BOOST_TEST(is_beta_normal(n));
    BOOST_TEST(beta_normalize(n) == n);
    BOOST_TEST(n.term() == pre_typed_term::abs("x", s, x));
}

BOOST_AUTO_TEST_CASE(should_reduce_consecutive_applications)
{
    context const ctx{{{pre_typed_term::var_t("y"), s}}};
    auto const m =
        legal_term(
            type_assign(
                ctx,
                pre_typed_term::app(
                    pre_typed_term::app(
                        pre_typed_term::abs("f", s_to_s, pre_typed_term::var("f")),
                        id),
                    y)
            ).value());
    BOOST_TEST(num_redexes(m) == 1ul);
    BOOST_TEST(not is_redex(m)); // it contains a redex, but it isn't one itself
    BOOST_TEST(not is_beta_normal(m));
    BOOST_TEST(beta_normalize(m) != m);

    auto const n = beta_normalize(m);
    BOOST_TEST(num_redexes(n) == 0ul);
    BOOST_TEST(not is_redex(n));
    BOOST_TEST(is_beta_normal(n));
    BOOST_TEST(beta_normalize(n) == n);
    BOOST_TEST(n.term() == y);
}

BOOST_AUTO_TEST_SUITE_END()
