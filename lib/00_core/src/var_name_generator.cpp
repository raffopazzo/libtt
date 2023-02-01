#include "libtt/core/var_name_generator.hpp"

#include <limits>

namespace libtt::core {

var_name_generator::iterator& var_name_generator::iterator::operator++()
{
    ++id;
    return *this;
}

var_name_generator::iterator var_name_generator::iterator::operator++(int)
{
    return iterator{id++};
}

std::string var_name_generator::iterator::operator*() const
{
    return "tmp_" + std::to_string(id);
}

var_name_generator::iterator var_name_generator::begin()
{
    return iterator{0ul};
}

var_name_generator::iterator var_name_generator::end()
{
    return iterator{std::numeric_limits<std::size_t>::max()};
}

}
