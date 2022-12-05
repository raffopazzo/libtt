#include "libtt/untyp/subterms.hpp"

namespace libtt::untyp {

struct subterms_visitor
{
    std::vector<term>& acc;

    void operator()(term::var_t const& v)
    {
	acc.emplace_back(v);
    }

    void operator()(term::app_t const& x)
    {
	acc.emplace_back(x);
	std::visit(subterms_visitor{acc}, x.left.get().value);
	std::visit(subterms_visitor{acc}, x.right.get().value);
    }

    void operator()(term::abs_t const& x)
    {
	acc.emplace_back(x);
	std::visit(subterms_visitor{acc}, x.body.get().value);
    }
};

struct proper_subterms_visitor
{
    std::vector<term>& acc;

    void operator()(term::var_t const& v)
    {
    }

    void operator()(term::app_t const& x)
    {
	std::visit(subterms_visitor{acc}, x.left.get().value);
	std::visit(subterms_visitor{acc}, x.right.get().value);
    }

    void operator()(term::abs_t const& x)
    {
	std::visit(subterms_visitor{acc}, x.body.get().value);
    }
};

std::vector<term> subterms(term const& x)
{
    std::vector<term> acc;
    std::visit(subterms_visitor{acc}, x.value);
    return acc;
}

std::vector<term> proper_subterms(term const& x)
{
    std::vector<term> acc;
    std::visit(proper_subterms_visitor{acc}, x.value);
    return acc;
}

}
