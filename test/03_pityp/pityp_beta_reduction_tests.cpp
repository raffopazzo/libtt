#define BOOST_TEST_MODULE pityp_beta_reduction
#include <boost/test/included/unit_test.hpp>

#include "libtt/pityp/beta_reduction.hpp"
#include "libtt/pityp/pretty_print.hpp"

//namespace libtt::typed {
//
//template <typename T, typename U>
//std::ostream& operator<<(std::ostream& os, std::pair<T, U> const& x)
//{
//    return os << x.first << ':' << x.second;
//}
//
//std::ostream& operator<<(std::ostream& os, type const& x) { return pretty_print(os, x); }
//std::ostream& operator<<(std::ostream& os, pre_typed_term::var_t const& x) { return os << x.name; }
//std::ostream& operator<<(std::ostream& os, pre_typed_term const& x) { return pretty_print(os, x); }
//std::ostream& operator<<(std::ostream& os, legal_term const& x)
//{
//    os << '{';
//    for (auto const& kv: x.ctx().decls)
//        os << kv;
//    os << '}';
//    return os << " => " << x.term() << " : " << x.ty();
//}
//
//}
//

namespace libtt::pityp {

//template <typename T>
//std::ostream& operator<<(std::ostream& os, std::optional<T> const& x)
//{
//    return x ? os << *x : os << "nullopt";
//}
//
//std::ostream& operator<<(std::ostream& os, context::type_decl_t const& x) { return pretty_print(os, x); }
//std::ostream& operator<<(std::ostream& os, context::var_decl_t const& x) { return pretty_print(os, x); }
std::ostream& operator<<(std::ostream& os, legal_term const& x) { return pretty_print(os, x); }
std::ostream& operator<<(std::ostream& os, type const& x) { return pretty_print(os, x); }
std::ostream& operator<<(std::ostream& os, pre_typed_term const& x) { return pretty_print(os, x); }
//
//template <typename T1, typename T2, typename... Ts>
//pre_typed_term app(T1 const& x1, T2 const& x2, Ts const&... xs)
//{
//    if constexpr (sizeof...(Ts) == 0)
//        return pre_typed_term::app(x1, x2);
//    else
//        return app(pre_typed_term::app(x1, x2), xs...);
//}

}

using namespace libtt::pityp;

struct pityp_beta_reduction_tests_fixture
{
    type const s = type::var("s");
    type const t = type::var("t");
    type const s_to_s = type::arr(s, s);
    context const empty;
    pre_typed_term const x = pre_typed_term::var("x");
    pre_typed_term const y = pre_typed_term::var("y");
    pre_typed_term const id = pre_typed_term::abs(pre_typed_term::var_t("z"), s, pre_typed_term::var("z"));
};

BOOST_FIXTURE_TEST_SUITE(pityp_beta_reduction_tests, pityp_beta_reduction_tests_fixture)

BOOST_AUTO_TEST_CASE(variable_is_beta_normal)
{
    context ctx;
    ctx = extend(ctx, type::var_t("s")).value();
    ctx = extend(ctx, pre_typed_term::var_t("x"), s).value();
    auto const m = legal_term(type_assign(ctx, x).value());
    BOOST_TEST(num_redexes(m) == 0ul);
    BOOST_TEST(not is_redex(m));
    BOOST_TEST(is_beta_normal(m));
    BOOST_TEST(beta_normalize(m) == m);
}

BOOST_AUTO_TEST_CASE(identity_is_beta_normal)
{
    context ctx;
    ctx = extend(ctx, type::var_t("s")).value();
    auto const m = legal_term(type_assign(ctx, id).value());
    BOOST_TEST(num_redexes(m) == 0ul);
    BOOST_TEST(not is_redex(m));
    BOOST_TEST(is_beta_normal(m));
    BOOST_TEST(beta_normalize(m) == m);
}

BOOST_AUTO_TEST_CASE(polimorphic_identity_is_beta_normal)
{
    auto const m = legal_term(type_assign(empty, pre_typed_term::abs(type::var_t("s"), id)).value());
    BOOST_TEST(num_redexes(m) == 0ul);
    BOOST_TEST(not is_redex(m));
    BOOST_TEST(is_beta_normal(m));
    BOOST_TEST(beta_normalize(m) == m);
}

BOOST_AUTO_TEST_CASE(identity_reduces_to_argument)
{
    context ctx;
    ctx = extend(ctx, type::var_t("s")).value();
    ctx = extend(ctx, pre_typed_term::var_t("x"), s).value();
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
    BOOST_TEST(n.ty() == s);
}

BOOST_AUTO_TEST_CASE(polimorphic_identity_reduces_to_argument)
{
    context ctx;
    ctx = extend(ctx, type::var_t("t")).value();
    ctx = extend(ctx, pre_typed_term::var_t("y"), t).value();
    auto const m =
        legal_term(
            type_assign(
                ctx,
                pre_typed_term::app(
                    pre_typed_term::app(pre_typed_term::abs(type::var_t("s"), id), t),
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
    BOOST_TEST(n.ty() == t);
}

// BOOST_AUTO_TEST_CASE(applications_with_function_from_context_dont_reduce)
// {
//     context const ctx{{{pre_typed_term::var_t("x"), s}, {pre_typed_term::var_t("f"), s_to_s}}};
//     auto const m = legal_term(type_assign(ctx, pre_typed_term::app(pre_typed_term::var("f"), x)).value());
//     BOOST_TEST(num_redexes(m) == 0ul);
//     BOOST_TEST(not is_redex(m));
//     BOOST_TEST(is_beta_normal(m));
//     BOOST_TEST(beta_normalize(m) == m);
// }
// 
// BOOST_AUTO_TEST_CASE(can_beta_reduce_recursively)
// {
//     auto const m = legal_term(type_assign(empty, pre_typed_term::abs("x", s, pre_typed_term::app(id, x))).value());
//     BOOST_TEST(num_redexes(m) == 1ul);
//     BOOST_TEST(not is_redex(m)); // it contains a redex, but it isn't one itself
//     BOOST_TEST(not is_beta_normal(m));
//     BOOST_TEST(beta_normalize(m) != m);
// 
//     auto const n = beta_normalize(m);
//     BOOST_TEST(num_redexes(n) == 0ul);
//     BOOST_TEST(not is_redex(n));
//     BOOST_TEST(is_beta_normal(n));
//     BOOST_TEST(beta_normalize(n) == n);
//     BOOST_TEST(n.term() == pre_typed_term::abs("x", s, x));
// }
// 
// BOOST_AUTO_TEST_CASE(beta_normalization_tests)
// {
//     auto const nat = type::var("nat");
//     context ctx;
//     ctx = extend(ctx, type::var_t("nat")).value();
//     ctx = extend(ctx, pre_typed_term::var_t("suc"), type::arr(nat, nat)).value();
//     auto const a = type::var("a");
//     auto const f = pre_typed_term::var("f");
//     auto const x = pre_typed_term::var("x");
//     auto const abs = [] <typename... Ts> (Ts&&... args) { return pre_typed_term::abs(std::forward<Ts>(args)...); };
//     auto const var_f = pre_typed_term::var_t("f");
//     auto const var_x = pre_typed_term::var_t("x");
//     // polymorphic function that applies any function f twice to any argument x
//     auto const twice = abs(type::var_t("a"), abs(var_f, type::arr(a, a), abs(var_x, a, app(f, app(f, x)))));
//     BOOST_TEST(legal_term(type_assign(ctx, twice).value()).ty() == type::pi("a", type::arr(type::arr(a, a), type::arr(a, a))));
//     ctx = extend(ctx, pre_typed_term::var_t("two"), nat).value();
//     auto const suc = pre_typed_term::var("suc");
//     auto const two = pre_typed_term::var("two");
//     auto const twice_nat_suc_two = legal_term(type_assign(ctx, app(twice, nat, suc, two)).value());
//     auto const suc_suc_two = legal_term(type_assign(ctx, app(suc, app(suc, two))).value());
// 
//     BOOST_TEST(beta_normalize(twice_nat_suc_two) == suc_suc_two);
// }

BOOST_AUTO_TEST_SUITE_END()
