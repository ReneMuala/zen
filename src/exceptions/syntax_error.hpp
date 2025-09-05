//
// Created by dte on 1/22/2025.
//

#pragma once
#include <stdexcept>
#include <fmt/format.h>

namespace zen::exceptions
{
    class syntax_error final : public std::logic_error
    {
    public:
        explicit syntax_error(const std::string& message, int offset);
    };
}
