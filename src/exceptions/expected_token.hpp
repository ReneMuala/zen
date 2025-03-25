//
// Created by dte on 1/22/2025.
//

#pragma once
#include <stdexcept>
#include <fmt/format.h>

namespace zen::exceptions
{
    class expected_token final : public std::invalid_argument
    {
    public:
        explicit expected_token(const std::string& token, const long& line, const long& col);
    };
}
