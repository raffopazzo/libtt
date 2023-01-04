#define BOOST_TEST_MODULE term_search
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

BOOST_AUTO_TEST_SUITE(term_search_tests)

BOOST_AUTO_TEST_CASE(term_search_test)
{
    auto const s = type::var("s");
    auto const t = type::var("t");
    auto const s_to_t = type::arr(s, t);
    auto const x = pre_typed_term::var("x");
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
        decls.emplace(pre_typed_term::var_t("f"), s_to_t);
        decls.emplace(pre_typed_term::var_t("u"), s);
        return decls;
    }());
    BOOST_TEST(not conclusion_of(term_search(empty, s)).has_value());
    BOOST_TEST(not conclusion_of(term_search(ctx, s)).has_value());
    BOOST_TEST(conclusion_of(term_search(ctx, t)) == judgement(ctx, statement(x, t)));
    auto const conclusion_1 = conclusion_of(term_search(ctx, s_to_t));
    BOOST_TEST(conclusion_1.has_value());
    if (conclusion_1.has_value())
    {
        BOOST_TEST(conclusion_1->stm.ty == s_to_t);
    }
    auto const conclusion_2 = conclusion_of(term_search(ctx2, t));
    BOOST_TEST(conclusion_2.has_value());
    if (conclusion_2.has_value())
    {
        BOOST_TEST(conclusion_2->stm.ty == t);
    }
}

BOOST_AUTO_TEST_SUITE_END()
