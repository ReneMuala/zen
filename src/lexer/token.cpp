//
// Created by dte on 1/22/2025.
//

#include "token.hpp"

namespace zen {
    token::token(const enums::token_type& type, std::string&& value, long line, long column): type(type), value(std::move(value)), line(line), column(column)
    {}

    token::token(const enums::token_type& type, long line, long column): type(type), line(line), column(column)
    {}

    // token::token(const enums::token_type & type, std::string && value): type(type)
    // {
    //     if (not value.empty())
    //         this->value = std::move(value);
    // }
} // zen