#define BOOST_TEST_MODULE pityp_context
#include <boost/test/included/unit_test.hpp>

#include "libtt/pityp/derivation.hpp"
#include "libtt/pityp/pretty_print.hpp"

namespace libtt::pityp {

template <typename T>
std::ostream& operator<<(std::ostream& os, std::optional<T> const& x)
{
    return x ? (os << *x) : (os << "nullopt");
}

std::ostream& operator<<(std::ostream& os, type const& x) { return pretty_print(os, x); }
std::ostream& operator<<(std::ostream& os, context::type_decl_t const& x) { return os << '*'; }
std::ostream& operator<<(std::ostream& os, context::var_decl_t const& x)
{
    return pretty_print(os << x.subject.name << " : ", x.ty);
}

}

using namespace libtt::pityp;

BOOST_AUTO_TEST_SUITE(pityp_context_tests)

BOOST_AUTO_TEST_CASE(default_context_is_empty)
{
    context const empty;
    BOOST_TEST(empty.empty());
}

BOOST_AUTO_TEST_CASE(can_add_type_decl)
{
    context const empty;
    auto const res1 = extend(empty, type::var_t("s"));
    BOOST_REQUIRE(res1.has_value());
    auto const& ctx1 = *res1;
    BOOST_TEST(not ctx1.empty());
    BOOST_TEST(ctx1.contains(type::var_t("s")));
    BOOST_TEST(not ctx1.contains(pre_typed_term::var_t("s")));
    BOOST_TEST(ctx1[type::var_t("s")] == context::type_decl_t(type::var_t("s")));

    auto const res2 = extend(ctx1, type::var_t("t"));
    BOOST_REQUIRE(res2.has_value());
    auto const& ctx2 = *res2;
    BOOST_TEST(not ctx2.empty());
    BOOST_TEST(ctx2.contains(type::var_t("s")));
    BOOST_TEST(ctx2.contains(type::var_t("t")));
    BOOST_TEST(not ctx2.contains(pre_typed_term::var_t("s")));
    BOOST_TEST(not ctx2.contains(pre_typed_term::var_t("t")));
    BOOST_TEST(ctx2[type::var_t("s")] == context::type_decl_t(type::var_t("s")));
    BOOST_TEST(ctx2[type::var_t("t")] == context::type_decl_t(type::var_t("t")));

    auto const res3 = extend(ctx2, type::var_t("t")); // allow idempotence
    BOOST_REQUIRE(res3.has_value());
    auto const& ctx3 = *res3;
    BOOST_TEST(not ctx3.empty());
    BOOST_TEST(ctx3.contains(type::var_t("s")));
    BOOST_TEST(ctx3.contains(type::var_t("t")));
    BOOST_TEST(not ctx3.contains(pre_typed_term::var_t("s")));
    BOOST_TEST(not ctx3.contains(pre_typed_term::var_t("t")));
    BOOST_TEST(ctx3[type::var_t("s")] == context::type_decl_t(type::var_t("s")));
    BOOST_TEST(ctx3[type::var_t("t")] == context::type_decl_t(type::var_t("t")));
}

BOOST_AUTO_TEST_CASE(can_add_var_decl)
{
    context const empty;
    BOOST_TEST(not extend(empty, pre_typed_term::var_t("x"), type::var("s")).has_value());
    auto const ctx1 = extend(empty, type::var_t("s"));
    BOOST_REQUIRE(ctx1.has_value());
    auto const res2 = extend(*ctx1, pre_typed_term::var_t("x"), type::var("s"));
    BOOST_REQUIRE(res2.has_value());
    auto const& ctx2 = *res2;
    BOOST_TEST(not ctx2.empty());
    BOOST_TEST(ctx2.contains(pre_typed_term::var_t("x")));
    BOOST_TEST(not ctx2.contains(type::var_t("x")));
    BOOST_TEST(ctx2[pre_typed_term::var_t("x")] == context::var_decl_t(pre_typed_term::var_t("x"), type::var("s")));
}

BOOST_AUTO_TEST_CASE(cannot_add_var_if_name_maps_to_type)
{
    context ctx;
    ctx = extend(ctx, type::var_t("s")).value();
    ctx = extend(ctx, type::var_t("t")).value();
    BOOST_TEST(not extend(ctx, pre_typed_term::var_t("s"), type::var("t")).has_value());
}

BOOST_AUTO_TEST_CASE(cannot_add_type_if_name_maps_to_var)
{
    context ctx;
    ctx = extend(ctx, type::var_t("s")).value();
    ctx = extend(ctx, pre_typed_term::var_t("t"), type::var("s")).value();
    BOOST_TEST(not extend(ctx, type::var_t("t")).has_value());
}

BOOST_AUTO_TEST_SUITE_END()
