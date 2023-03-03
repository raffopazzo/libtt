#define BOOST_TEST_MODULE pi_type
#include <boost/test/included/unit_test.hpp>

#include "libtt/pityp/type.hpp"
#include "libtt/pityp/pretty_print.hpp"

namespace libtt::pityp {

std::ostream& operator<<(std::ostream&, type::var_t const&);
std::ostream& operator<<(std::ostream&, type::arr_t const&);
std::ostream& operator<<(std::ostream&, type::pi_t const&);
std::ostream& operator<<(std::ostream&, type const&);

std::ostream& operator<<(std::ostream& os, type::var_t const& x)
{
    return os << x.name;
}

std::ostream& operator<<(std::ostream& os, type::arr_t const& x)
{
    return os << "((" << x.dom.get() << ") -> (" << x.img.get() << "))";
}

std::ostream& operator<<(std::ostream& os, type::pi_t const& x)
{
    return os << "(pi " << x.var << " : * . " << x.body.get() << ')';
}

std::ostream& operator<<(std::ostream& os, type const& x)
{
    return std::visit([&os](auto const& x) -> std::ostream& {return os << x;}, x.value);
}

}

using namespace libtt::pityp;

BOOST_AUTO_TEST_SUITE(pi_type_tests)

BOOST_AUTO_TEST_CASE(type_variable_tests)
{
    auto const s = type::var("s");
    auto const t = type::var("t");

    BOOST_TEST(s == s);
    BOOST_TEST(t == t);
    BOOST_TEST(s != t);
    BOOST_TEST(t != s);
    BOOST_REQUIRE(is_var(s));
    BOOST_REQUIRE(is_var(t));
    BOOST_TEST(not is_arr(s));
    BOOST_TEST(not is_arr(t));
    BOOST_TEST(not is_pi(s));
    BOOST_TEST(not is_pi(t));
    BOOST_TEST(std::get<type::var_t>(s.value).name == "s");
    BOOST_TEST(std::get<type::var_t>(t.value).name == "t");
}

BOOST_AUTO_TEST_CASE(arrow_type_tests)
{
    auto const s = type::var("s");
    auto const t = type::var("t");
    auto const f = type::arr(s, t);
    auto const g = type::arr(f, t);
    auto const h = type::arr(t, f);
    auto const j = type::arr(g, h);

    BOOST_TEST(f == f);
    BOOST_TEST(f != g);
    BOOST_TEST(f != h);
    BOOST_TEST(f != j);
    BOOST_TEST(not is_var(f));
    BOOST_REQUIRE(is_arr(f));
    BOOST_TEST(not is_pi(f));
    BOOST_TEST(std::get<type::arr_t>(f.value).dom.get() == s);
    BOOST_TEST(std::get<type::arr_t>(f.value).img.get() == t);

    BOOST_TEST(g != f);
    BOOST_TEST(g == g);
    BOOST_TEST(g != h);
    BOOST_TEST(g != j);
    BOOST_TEST(not is_var(g));
    BOOST_REQUIRE(is_arr(g));
    BOOST_TEST(not is_pi(g));
    BOOST_TEST(std::get<type::arr_t>(g.value).dom.get() == f);
    BOOST_TEST(std::get<type::arr_t>(g.value).img.get() == t);

    BOOST_TEST(h != f);
    BOOST_TEST(h != g);
    BOOST_TEST(h == h);
    BOOST_TEST(h != j);
    BOOST_TEST(not is_var(h));
    BOOST_REQUIRE(is_arr(h));
    BOOST_TEST(not is_pi(h));
    BOOST_TEST(std::get<type::arr_t>(h.value).dom.get() == t);
    BOOST_TEST(std::get<type::arr_t>(h.value).img.get() == f);

    BOOST_TEST(j != f);
    BOOST_TEST(j != g);
    BOOST_TEST(j != h);
    BOOST_TEST(j == j);
    BOOST_TEST(not is_var(j));
    BOOST_REQUIRE(is_arr(j));
    BOOST_TEST(not is_pi(h));
    BOOST_TEST(std::get<type::arr_t>(j.value).dom.get() == g);
    BOOST_TEST(std::get<type::arr_t>(j.value).img.get() == h);
}

BOOST_AUTO_TEST_CASE(pi_type_tests)
{
    auto const s = type::var("s");
    auto const t = type::var("t");
    auto const p = type::pi("s", type::arr(s, s));
    auto const q = type::pi("t", type::arr(s, t));
    auto const r = type::pi("s", type::pi("t", type::arr(s, t)));

    BOOST_TEST(p == p);
    BOOST_TEST(p != q);
    BOOST_TEST(p != r);
    BOOST_TEST(not is_var(p));
    BOOST_TEST(not is_arr(p));
    BOOST_REQUIRE(is_pi(p));
    BOOST_TEST(std::get<type::pi_t>(p.value).var.name == "s");
    BOOST_TEST(std::get<type::pi_t>(p.value).body.get() == type::arr(s, s));

    BOOST_TEST(q != p);
    BOOST_TEST(q == q);
    BOOST_TEST(q != r);
    BOOST_TEST(not is_var(q));
    BOOST_TEST(not is_arr(q));
    BOOST_REQUIRE(is_pi(q));
    BOOST_TEST(std::get<type::pi_t>(q.value).var.name == "t");
    BOOST_TEST(std::get<type::pi_t>(q.value).body.get() == type::arr(s, t));

    BOOST_TEST(r != p);
    BOOST_TEST(r != q);
    BOOST_TEST(r == r);
    BOOST_TEST(not is_var(r));
    BOOST_TEST(not is_arr(r));
    BOOST_REQUIRE(is_pi(r));
    BOOST_TEST(std::get<type::pi_t>(r.value).var.name == "s");
    BOOST_TEST(std::get<type::pi_t>(r.value).body.get() == q);
}

BOOST_AUTO_TEST_CASE(pretty_print_tests)
{
    auto const s = type::var("s");
    auto const t = type::var("t");
    auto const f = type::arr(s, t);
    auto const g = type::arr(f, t);
    auto const h = type::arr(t, f);
    auto const j = type::arr(g, h);
    auto const k = type::arr(g, g);
    auto const p = type::pi("s", type::arr(s, s));
    auto const q = type::pi("t", type::arr(s, t));
    auto const r = type::pi("s", type::pi("t", type::arr(s, t)));

    auto const to_str = [] (type const& x)
    {
        std::ostringstream buf;
        pretty_print(buf, x);
        return buf.str();
    };

    BOOST_TEST(to_str(s) == "s");
    BOOST_TEST(to_str(t) == "t");
    BOOST_TEST(to_str(f) == "s -> t");
    BOOST_TEST(to_str(g) == "(s -> t) -> t");
    BOOST_TEST(to_str(h) == "t -> s -> t");
    BOOST_TEST(to_str(j) == "((s -> t) -> t) -> t -> s -> t");
    BOOST_TEST(to_str(k) == "((s -> t) -> t) -> (s -> t) -> t");
    BOOST_TEST(to_str(p) == "pi s : * . s -> s");
    BOOST_TEST(to_str(q) == "pi t : * . s -> t");
    BOOST_TEST(to_str(r) == "pi s, t : * . s -> t");
}

BOOST_AUTO_TEST_SUITE_END()
