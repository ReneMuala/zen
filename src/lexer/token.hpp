//
// Created by dte on 1/22/2025.
//

#pragma once
#include "enums/token_type.hpp"
#include <string>

namespace zen {
    struct token {
        const enums::token_type type;
        std::string value;
        long line, column;

        [[nodiscard]] inline std::string get_location_string() const
        {
            return std::to_string(line) + ":" + std::to_string(column);
        }

        explicit token(const enums::token_type& type, std::string&& value , long line = 0, long column = 0);
        explicit token(const enums::token_type& type, long line = 0, long column = 0);
    };
} // zen