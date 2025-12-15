//
// Created by dte on 1/22/2025.
//

#include "link_error.hpp"

#include <vector>

#include "lexer/token.hpp"
#include "types/stack.hpp"

extern std::vector<zen::token> tokens;

namespace zen::exceptions
{
    link_error::link_error(const std::string& message): logic_error(fmt::format(R"([link error]: {})", message)){}
}
