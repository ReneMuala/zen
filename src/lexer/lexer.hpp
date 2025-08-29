//
// Created by dte on 1/22/2025.
//

#pragma once
#include <fstream>
#include <list>
#include <optional>
#include <string>
#include <exceptions/file_not_found.hpp>
#include "token.hpp"

namespace zen {
    class lexer {
        std::ifstream stream;
        char lit, it;
        long line, col;
        inline void getchar();
        public:
        explicit lexer(const std::string&& filename);

        std::optional<token> next();
        inline token next_int_or_double();
        inline token next_string();
        zen::token next_and();
        zen::token next_or();
        inline token next_equ_or_equal();
        inline token next_single(const enums::token_type && type);
        inline token next_not_or_not_equal();
        inline token next_greater_or_greater_equal();
        inline token next_lower_or_lower_equal();
        inline token next_plus_or_plus_plus_or_plus_equal();
        inline token next_minus_or_minus_minus_or_minus_equal();
        inline token next_times_or_times_equal();
        inline token next_modulo_or_modulo_equal();
        inline token next_slash_or_slash_equal_or_comment();
        inline token next_id_or_keyword();
        inline token next_error();
    };
}
