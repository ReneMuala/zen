//
// Created by dte on 1/22/2025.
//

#include "semantic_error.hpp"

#include <vector>

#include "lexer/token.hpp"

extern std::vector<zen::token> tokens;

namespace zen::exceptions
{
    semantic_error::semantic_error(const std::string& message, const int offset, const std::string & hint): logic_error(
            hint.empty() ? fmt::format(R"([semantic error]: {} near {})", message, tokens.empty() ? "0:0" : tokens[std::max(offset - 2, 0)].get_location_string()) :
            fmt::format("[semantic error]: {} near {}\n\t{}", message, tokens.empty() ? "0:0" : tokens[std::max(offset - 2, 0)].get_location_string(), hint)
    ){}
}
