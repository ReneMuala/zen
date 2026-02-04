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
        const std::unordered_map<std::string, std::shared_ptr<builder::type>> & generic_context_mapping;
        const std::shared_ptr<builder::function> & function;
        std::shared_ptr<builder::type> type;
        std::shared_ptr<builder::program> program;
        static std::vector<std::string> split_name(const std::string& name);
        static std::expected<std::shared_ptr<value>, std::string> get_field(std::shared_ptr<value>, const std::vector<std::string>& tokens, const std::function<void(std::shared_ptr<value>&, const
                                                                                std
                                                                                ::shared_ptr<value>&)>& pointer_handler);
        std::shared_ptr<builder::value> get_field_or_throw(const std::shared_ptr<builder::value>& object,
                                                           const std::string& field) const;
        std::expected<std::shared_ptr<value>, std::string> get_value(const std::string& name, const std::function<void(std::shared_ptr<value>&, const std
                                                                         ::shared_ptr<value>&)>& pointer_handler);
        std::expected<std::shared_ptr<value>, std::string> get_value(const std::string& name);
        static std::string simple_name(const std::string& name);
        static std::string resolve_type_name(const std::string& name,  const std::unordered_map<std::string, std::shared_ptr<builder::type>> & gcm = {});
        static const std::string extract_specified_lib(std::string& name, std::shared_ptr<library>& specified_lib, std::shared_ptr<builder::program> program);
        std::expected<std::pair<std::shared_ptr<builder::value>,std::shared_ptr<builder::function>>, std::string> get_function(std::string name, std::vector<std::shared_ptr<zen::builder::type>>& params, std::string& hint);
        std::expected<std::shared_ptr<generic_context>, std::string> get_generic_function_or_type(
            std::string name, size_t param_count);
        static std::expected<std::shared_ptr<generic_context>, std::string> get_generic_type(
            std::string name, size_t param_count, const std::shared_ptr<builder::program>& program);
        std::expected<std::pair<std::shared_ptr<builder::value>,std::shared_ptr<builder::function>>, std::string> get_function(const std::shared_ptr<zen::builder::value>& object, const std::string& name, std::vector<std::shared_ptr<zen::builder::
                                                                                                                                   type>>& params, std::string& hint);
        // std::expected<std::shared_ptr<builder::type>, std::string> get_type(const std::string& name);
        static std::expected<std::vector<std::shared_ptr<builder::type>>, std::string> get_types(const std::vector<std::string> & types, const std::shared_ptr<builder::program>& program,  const std::unordered_map<std::string, std::shared_ptr<builder::type>> & gcm);
        static std::expected<std::shared_ptr<builder::type>, std::string> get_type(const std::string& name, const std::shared_ptr<builder::program>& program,  const std::unordered_map<std::string, std::shared_ptr<builder::type>> & gcm = {});
        static std::shared_ptr<table> create(const std::shared_ptr<builder::function>& function, const std::unordered_map<std::string, std::shared_ptr<builder::type>> & gcm = {}, const std::shared_ptr<builder::type>& type = nullptr, const std::shared_ptr<builder::program>& program = nullptr);
    };
    namespace table_helpers
    {
        struct scope
        {
            table& tab;
            scope(table& tab, std::string & name);
            ~scope();
        };
    }
}
