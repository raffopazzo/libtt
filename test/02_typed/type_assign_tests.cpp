#define BOOST_TEST_MODULE type_assign
#include <boost/test/included/unit_test.hpp>

#include "libtt/typed/derivation.hpp"
#include "libtt/typed/pretty_print.hpp"

namespace libtt::typed {

template <typename T>
std::ostream& operator<<(std::ostream& os, std::optional<T> const& x)
{
    return x ? (os << *x) : (os << "nullopt");
}

template <typename T, typename U>
std::ostream& operator<<(std::ostream& os, std::pair<T, U> const& x)
{
    return os << x.first << ':' << x.second;
}

std::ostream& operator<<(std::ostream& os, type const& t) { return pretty_print(os, t); };

std::ostream& operator<<(std::ostream&, pre_typed_term::var_t const&);
std::ostream& operator<<(std::ostream&, pre_typed_term::app_t const&);
std::ostream& operator<<(std::ostream&, pre_typed_term::abs_t const&);
std::ostream& operator<<(std::ostream&, pre_typed_term const&);

std::ostream& operator<<(std::ostream& os, pre_typed_term::var_t const& x)
{
    return os << x.name;
}

std::ostream& operator<<(std::ostream& os, pre_typed_term::app_t const& x)
{
    return os << '(' << x.left.get() << ' ' << x.right.get() << ')';
}

std::ostream& operator<<(std::ostream& os, pre_typed_term::abs_t const& x)
{
    return os << "(lambda " << x.var << " : " << x.var_type << " . " << x.body.get() << ')';
}

std::ostream& operator<<(std::ostream& os, pre_typed_term const& x)
{
    return std::visit([&os](auto const& x) -> std::ostream& {return os << x;}, x.value);
}

std::ostream& operator<<(std::ostream& os, statement const& x)
{
    return os << x.subject << ": " << x.ty;
}

std::ostream& operator<<(std::ostream& os, judgement const& x)
{
    bool comma = false;
    for (auto const& [var, ty] : x.ctx.decls)
        (comma ? os << ", " : os) << var << ": " << ty;
    return os << " => " << x.stm;
}

}

using namespace libtt::typed;

BOOST_AUTO_TEST_SUITE(type_assign_tests)

BOOST_AUTO_TEST_CASE(type_assign_var_test)
{
    auto const s = type::var("s");
    auto const t = type::var("t");
    auto const s_to_s = type::arr(s, s);
    auto const s_to_t = type::arr(s, t);
    auto const x = pre_typed_term::var("x");
    auto const y = pre_typed_term::var("y");
    auto const xx = pre_typed_term::app(x, x);
    auto const xy = pre_typed_term::app(x, y);
    auto const f = pre_typed_term::abs("y", s, x);
    auto const g = pre_typed_term::abs("x", s, x);
    auto const fx = pre_typed_term::app(f, x);
    auto const fy = pre_typed_term::app(f, y);
    auto const gx = pre_typed_term::app(g, x);
    auto const gy = pre_typed_term::app(g, y);

    auto const empty = context{};
    auto const ctx = context([&]
    {
        std::map<pre_typed_term::var_t, type> decls;
        decls.emplace(pre_typed_term::var_t("x"), t);
        return decls;
    }());
    auto const ctx2 = context([&]
    {
        std::map<pre_typed_term::var_t, type> decls;
        decls.emplace(pre_typed_term::var_t("y"), s);
        return decls;
    }());

    BOOST_TEST(conclusion_of(type_assign(ctx, x)) == judgement(ctx, statement(x, t)));
    BOOST_TEST(not conclusion_of(type_assign(ctx, xx)).has_value());
    BOOST_TEST(not conclusion_of(type_assign(ctx, xy)).has_value());
    BOOST_TEST(conclusion_of(type_assign(ctx, f)) == judgement(ctx, statement(f, s_to_t)));
    BOOST_TEST(conclusion_of(type_assign(ctx, g)) == judgement(ctx, statement(g, s_to_s)));
    BOOST_TEST(not conclusion_of(type_assign(empty, f)).has_value());
    BOOST_TEST(conclusion_of(type_assign(empty, g)) == judgement(empty, statement(g, s_to_s)));
    BOOST_TEST(not conclusion_of(type_assign(ctx, fx)).has_value());
    BOOST_TEST(not conclusion_of(type_assign(ctx, fy)).has_value());
    BOOST_TEST(not conclusion_of(type_assign(ctx2, fx)).has_value());
    BOOST_TEST(not conclusion_of(type_assign(ctx2, fy)).has_value());
    BOOST_TEST(not conclusion_of(type_assign(ctx, gx)).has_value());
    BOOST_TEST(not conclusion_of(type_assign(ctx, gy)).has_value());
    BOOST_TEST(conclusion_of(type_assign(ctx2, gy)) == judgement(ctx2, statement(gy, s)));
    BOOST_TEST(not conclusion_of(type_assign(empty, gx)).has_value());
    BOOST_TEST(not conclusion_of(type_assign(empty, gy)).has_value());
}

BOOST_AUTO_TEST_CASE(exercise_2_9)
{
    auto const a = type::var("a");
    auto const b = type::var("b");
    auto const c = type::var("c");
    auto const d = type::var("d");
    auto const ctx = context([&]
    {
        std::map<pre_typed_term::var_t, type> decls;
        decls.emplace(pre_typed_term::var_t("x"), type::arr(d, type::arr(d, a)));
        decls.emplace(pre_typed_term::var_t("y"), type::arr(c, a));
        decls.emplace(pre_typed_term::var_t("z"), type::arr(a, b));
        return decls;
    }());
    auto const d_c_b = type::arr(d, type::arr(c, b));
    auto const u = pre_typed_term::var("u");
    auto const v = pre_typed_term::var("v");
    auto const x = pre_typed_term::var("x");
    auto const y = pre_typed_term::var("y");
    auto const z = pre_typed_term::var("z");
    auto const abs = [] (auto... args) { return pre_typed_term::abs(args...); };
    auto const app = [] (auto... args) { return pre_typed_term::app(args...); };
    auto const expr_a = abs("u", d, abs("v", c, app(z, app(y, v))));
    auto const expr_b = abs("u", d, abs( "v", c, app(z, app(app(x, u), u))));
    BOOST_TEST(conclusion_of(type_assign(ctx, expr_a)) == judgement(ctx, statement(expr_a, d_c_b)));
    BOOST_TEST(conclusion_of(type_assign(ctx, expr_b)) == judgement(ctx, statement(expr_b, d_c_b)));
}

BOOST_AUTO_TEST_SUITE_END()
