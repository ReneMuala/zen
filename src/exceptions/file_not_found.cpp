//
// Created by dte on 1/22/2025.
//

#include "file_not_found.hpp"

namespace zen::exceptions
{
    file_not_found::file_not_found(const std::string& file): invalid_argument(fmt::format("[fatal]: file not found {}", file))
    {}
}
