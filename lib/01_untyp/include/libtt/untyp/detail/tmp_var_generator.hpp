#pragma once

#include "libtt/untyp/term.hpp"

#include <cstddef>

namespace libtt::untyp::detail {

struct tmp_var_generator
{
    struct iterator
    {
        std::size_t id;

        iterator& operator++();
        iterator operator++(int);
        term::var_t operator*() const;

        bool operator==(iterator const&) const = default;
    };

    iterator begin();
    iterator end();
};

}
