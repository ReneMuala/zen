//
// Created by dte on 1/22/2025.
//

#include "syntax_error.hpp"

#include <vector>

#include "lexer/token.hpp"

extern std::vector<zen::token> tokens;

namespace zen::exceptions
{
    syntax_error::syntax_error(const std::string& message, const int offset): logic_error(
        fmt::format(R"([syntax error]: expected "{}" found "{}" at {})", message, tokens.empty() ? "0:0" : tokens[offset].value, tokens[offset-1].get_location_string()))
    {}
}
