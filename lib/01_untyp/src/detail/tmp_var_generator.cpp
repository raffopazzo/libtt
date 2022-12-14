#include "libtt/untyp/detail/tmp_var_generator.hpp"

#include <limits>

namespace libtt::untyp::detail {

tmp_var_generator::iterator& tmp_var_generator::iterator::operator++()
{
    ++id;
    return *this;
}

tmp_var_generator::iterator tmp_var_generator::iterator::operator++(int)
{
    return iterator{id++};
}

term::var_t tmp_var_generator::iterator::operator*() const
{
    return term::var_t("tmp_" + std::to_string(id));
}

tmp_var_generator::iterator tmp_var_generator::begin()
{
    return iterator{0ul};
}

tmp_var_generator::iterator tmp_var_generator::end()
{
    return iterator{std::numeric_limits<std::size_t>::max()};
}

}
