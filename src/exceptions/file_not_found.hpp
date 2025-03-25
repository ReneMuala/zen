//
// Created by dte on 1/22/2025.
//

#pragma once
#include <stdexcept>
#include <fmt/core.h>

namespace zen::exceptions {
    class file_not_found final : public std::invalid_argument
    {
    public:
        explicit file_not_found(const std::string & file);
    };
}
