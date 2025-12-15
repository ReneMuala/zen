//
// Created by dte on 1/22/2025.
//

#pragma once
#include <stdexcept>
#include <fmt/format.h>

#include "types/stack.hpp"

namespace zen::exceptions
{
    class link_error final : public std::logic_error
    {
    public:
        explicit link_error(const std::string& message);
    };
}
