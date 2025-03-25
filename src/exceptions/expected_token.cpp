//
// Created by dte on 1/22/2025.
//

#include "expected_token.hpp"

namespace zen::exceptions
{
    expected_token::expected_token(const std::string& token, const long& line, const long& col): invalid_argument(
        fmt::format("[fatal]: expected token {} at {}:{}", token, line, col))
    {}
}
