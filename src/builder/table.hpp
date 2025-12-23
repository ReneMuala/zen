//
// Created by dte on 12/11/2025.
//

#pragma once
#include <expected>
#include <memory>

#include "function.hpp"
#include "library.hpp"
#include "program.hpp"

namespace zen::builder
{
    struct table
    {
        const std::shared_ptr<builder::function> & function;
        std::shared_ptr<builder::type> type;
        std::shared_ptr<builder::program> program;
        static std::vector<std::string> split_name(const std::string& name);
        static std::expected<std::shared_ptr<value>, std::string> get_field(std::shared_ptr<value>, const std::vector<std::string>& tokens, const std::function<void(std::shared_ptr<value>&, const
                                                                                std
                                                                                ::shared_ptr<value>&)>& pointer_handler);
        std::expected<std::shared_ptr<value>, std::string> get_value(const std::string& name, const std::function<void(std::shared_ptr<value>&, const std
                                                                         ::shared_ptr<value>&)>& pointer_handler);
        std::expected<std::shared_ptr<value>, std::string> get_value(const std::string& name);
        std::expected<std::shared_ptr<builder::function>, std::string> get_function(const std::string& name, const std::vector<std::shared_ptr<zen::builder::type>> & params);
        std::expected<std::shared_ptr<builder::type>, std::string> get_type(const std::string& name);
        static std::shared_ptr<table> create(const std::shared_ptr<builder::function>& function, const std::shared_ptr<builder::type>& type = nullptr, const std::shared_ptr<builder::program>& program = nullptr);
    };
}
