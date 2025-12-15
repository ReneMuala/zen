//
// Created by dte on 12/11/2025.
//

#pragma once
#include <expected>
#include <memory>

#include "function.hpp"

namespace zen::builder
{
    struct table
    {
        const std::shared_ptr<builder::function> & function;
        std::shared_ptr<builder::type> type;

        std::expected<std::shared_ptr<value>, std::string> get_value(const std::string& name);
        static std::shared_ptr<table> create(const std::shared_ptr<builder::function>& function, std::shared_ptr<builder::type> type = nullptr);
    };
}
