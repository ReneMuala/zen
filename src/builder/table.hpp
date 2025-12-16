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
        static std::expected<std::shared_ptr<value>, std::string> get_field(std::shared_ptr<value>,const std::vector<std::string>& tokens, const std::function<std::shared_ptr<value>()>& pointer_creator);
        std::expected<std::shared_ptr<value>, std::string> get_value(const std::string& name, const std::function<std::shared_ptr<value>()>& pointer_creator);
        std::expected<std::shared_ptr<value>, std::string> get_value(const std::string& name);
        static std::shared_ptr<table> create(const std::shared_ptr<builder::function>& function, std::shared_ptr<builder::type> type = nullptr);
    };
}
