//
// Created by dte on 12/3/2025.
//

#pragma once

#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "function.hpp"
#include "type.hpp"
#include "fmt/core.h"
#include "types/stack.hpp"

namespace zen::builder
{
    struct generic_context
    {
        enum kind
        {
            type,
            function,
        };
        std::string name;
        long long offset;
        size_t chain_size;
        std::shared_ptr<zen::builder::type> class_;
        std::vector<std::string> params;

        void add_parameter(const std::string& parameter)
        {
            params.push_back(parameter);
        }

        types::stack::i64 hash() const;

        static std::shared_ptr<generic_context> create(const std::string& name, const long long offset, size_t chain_size, std::vector<std::string>& params, const std::shared_ptr<zen::builder::type>& class_ = nullptr)
        {
            auto t = std::make_shared<generic_context>(name, offset, chain_size);
            t->params = std::move(params);
            t->class_ = class_;
            return t;
        }

        static std::string get_name(const std::string& name, const std::vector<std::string> & arguments)
        {
            std::string args_string = name+"<";
            for (auto& argument : arguments)
            {
                args_string += fmt::format("{},", argument);
            }
            if (not arguments.empty())
                args_string.pop_back();
            args_string += ">";
            return args_string;
        }

        static types::stack::i64 get_hash(const std::string& name, const size_t param_count)
        {
            static constexpr std::hash<std::string> hasher;
            return hasher(name.substr(0, name.find_first_of('<'))+std::to_string(param_count));
        }

        std::expected<std::unordered_map<std::string, std::shared_ptr<builder::type>>, std::string> get_mapping(const std::vector<std::shared_ptr<builder::type>>& arguments) const
        {
            if (arguments.size() != params.size())
                return std::unexpected("mismatching number of arguments for generic type params");
            std::unordered_map<std::string, std::shared_ptr<builder::type>> mapping;
            for (int i = 0; i < arguments.size(); i++)
            {
                mapping[params[i]] = arguments[i];
            }
            return mapping;
        }

        std::expected<bool, std::string> implement(const std::vector<std::shared_ptr<builder::type>>& arguments, const std::function<void(const std::unordered_map<std::string, std::shared_ptr<builder::type>>& mapping, long long offset, size_t chain_size, const std::shared_ptr<zen::builder::type>& class_)>& implementer) const
        {
            std::unordered_map<std::string, std::shared_ptr<builder::type>> mapping;
            if (const auto result = get_mapping(arguments); result.has_value())
            {
                mapping = result.value();
            } else
            {
                return std::unexpected(result.error());
            }
            implementer(mapping, offset, chain_size, class_);
            return true;
        }
    };
}

