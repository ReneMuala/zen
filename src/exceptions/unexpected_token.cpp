//
// Created by dte on 1/22/2025.
//

#include "unexpected_token.hpp"

namespace zen::exceptions
{
    unexpected_token::unexpected_token(const std::string& token, const long& line, const long& col): invalid_argument(
        fmt::format("[fatal]: unexpected token {} at {}:{}", token, line, col))
    {}
}
