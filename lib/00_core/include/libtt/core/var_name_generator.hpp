#pragma once

#include <string>

namespace libtt::core {

struct var_name_generator
{
    struct iterator
    {
        std::size_t id;

        iterator& operator++();
        iterator operator++(int);
        std::string operator*() const;

        bool operator==(iterator const&) const = default;
    };

    iterator begin();
    iterator end();
};

}
