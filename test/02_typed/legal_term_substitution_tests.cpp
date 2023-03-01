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

struct legal_term_substitution_tests_fixture
{
    type const s = type::var("s");
    type const t = type::var("t");
    type const s_to_s = type::arr(s, s);
    context const empty;
    pre_typed_term const x = pre_typed_term::var("x");
    pre_typed_term const y = pre_typed_term::var("y");
    pre_typed_term const id = pre_typed_term::abs("z", s, pre_typed_term::var("z"));
};

BOOST_FIXTURE_TEST_SUITE(legal_term_substitution_tests, legal_term_substitution_tests_fixture)

BOOST_AUTO_TEST_CASE(if_y_not_in_context_of_m_it_should_fail)
{
    context const ctx1{{{pre_typed_term::var_t("x"), s}}};
    context const ctx2{{{pre_typed_term::var_t("x"), s}, {pre_typed_term::var_t("y"), s}}};

    auto const m = legal_term(type_assign(ctx1, x).value());
    auto const n = legal_term(type_assign(ctx2, y).value());

    BOOST_TEST(not substitute(m, pre_typed_term::var_t("x"), n).has_value());
}

BOOST_AUTO_TEST_CASE(if_z_not_free_var_in_m_it_should_succeed_without_changes)
{
    context const ctx{{{pre_typed_term::var_t("x"), s}, {pre_typed_term::var_t("y"), s}}};

    auto const m = legal_term(type_assign(ctx, x).value());
    auto const n = legal_term(type_assign(ctx, y).value());

    // `z` is not a free variable in `m`, so substitution should succeed without changes
    auto const result = substitute(m, pre_typed_term::var_t("z"), n);
    BOOST_TEST(result.has_value());
    if (result)
    {
        BOOST_TEST(result->ctx().decls == ctx.decls, boost::test_tools::per_element{});
        BOOST_TEST(result->term() == x);
        BOOST_TEST(result->ty() == s);
    }
}

BOOST_AUTO_TEST_CASE(if_y_in_n_has_different_type_from_x_in_m_it_should_fail)
{
    context const ctx1{{{pre_typed_term::var_t("x"), s}, {pre_typed_term::var_t("y"), s}}};
    context const ctx2{{{pre_typed_term::var_t("y"), t}}};

    auto const m = legal_term(type_assign(ctx1, x).value());
    auto const n = legal_term(type_assign(ctx2, y).value());

    // `y` in `n` has a different type from `x` in `m`, so substitution should fail
    BOOST_TEST(not substitute(m, pre_typed_term::var_t("x"), n).has_value());
}

BOOST_AUTO_TEST_CASE(if_m_and_n_have_same_type_it_should_succeed_and_remove_x_from_context)
{
    context const ctx{{{pre_typed_term::var_t("x"), s}, {pre_typed_term::var_t("y"), s}}};

    auto const m = legal_term(type_assign(ctx, x).value());
    auto const n = legal_term(type_assign(ctx, y).value());

    // `x` can be replaced with `y` because they have the same type, and the resulting context no longer needs `x`
    auto const result = substitute(m, pre_typed_term::var_t("x"), n);
    BOOST_TEST(result.has_value());
    if (result)
    {
        context const resulting_context{{{pre_typed_term::var_t("y"), s}}};
        BOOST_TEST(result->ctx().decls == resulting_context.decls, boost::test_tools::per_element{});
        BOOST_TEST(result->term() == y);
        BOOST_TEST(result->ty() == s);
    }
}

BOOST_AUTO_TEST_CASE(if_x_appears_again_in_n_it_should_fail)
{
    context const ctx{{{pre_typed_term::var_t("x"), s}, {pre_typed_term::var_t("id"), s_to_s}}};

    // `x` cannot be replaced by another term which requires `x` again, eg `id(x)`, so this should fail
    auto const m = legal_term(type_assign(ctx, x).value());
    auto const n = legal_term(type_assign(ctx, pre_typed_term::app(id, x)).value());
    BOOST_TEST(not substitute(m, pre_typed_term::var_t("x"), n).has_value());
}

BOOST_AUTO_TEST_CASE(substitution_tests)
{
    context const ctx{{
        {pre_typed_term::var_t("x"), s},
        {pre_typed_term::var_t("y"), s},
        {pre_typed_term::var_t("id"), s_to_s}
    }};

    auto const m = legal_term(type_assign(ctx, x).value());
    auto const n = legal_term(type_assign(ctx, pre_typed_term::app(id, y)).value());

    // `x` can however be replaced by `id y` and the resulting context no longer needs `x`
    auto const result = substitute(m, pre_typed_term::var_t("x"), n);
    BOOST_TEST(result.has_value());
    if (result)
    {
        context const resulting_context{{{pre_typed_term::var_t("y"), s}, {pre_typed_term::var_t("id"), s_to_s}}};
        BOOST_TEST(result->ctx().decls == resulting_context.decls, boost::test_tools::per_element{});
        BOOST_TEST(result->term() == pre_typed_term::app(id, y));
        BOOST_TEST(result->ty() == s);
    }
}

BOOST_AUTO_TEST_SUITE_END()
