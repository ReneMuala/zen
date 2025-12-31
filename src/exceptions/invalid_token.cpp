//
// Created by dte on 1/22/2025.
//

#include "invalid_token.hpp"

namespace zen::exceptions
{
    invalid_token::invalid_token(const std::string& token, const long& line, const long& col): invalid_argument(
        fmt::format("[lexical error]: invalid token {} at {}:{}", token, line, col))
    {}
}
