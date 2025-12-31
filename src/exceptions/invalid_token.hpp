//
// Created by dte on 1/22/2025.
//

#pragma once
#include <stdexcept>
#include <fmt/format.h>

namespace zen::exceptions
{
    class invalid_token final : public std::invalid_argument
    {
    public:
        explicit invalid_token(const std::string& token, const long& line, const long& col);
    };
}
