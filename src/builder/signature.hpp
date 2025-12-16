//
// Created by dte on 12/3/2025.
//
#pragma once

#include <memory>
#include <vector>
#include "type.hpp"
#include "value.hpp"
namespace zen::builder
{
    struct signature
    {
        std::shared_ptr<builder::type> type;
        std::vector<std::shared_ptr<builder::type>> parameters {};
        bool is_allocator = false;

        [[nodiscard]] bool check_args(const std::vector<std::shared_ptr<builder::value>>& args) const
        {
            if (args.size() != parameters.size())
            {
                return false;
            }

            for (int i = 0; i < parameters.size(); i++)
            {
                if (*parameters[i] != *args[i]->type)
                {
                    return false;
                }
            }
            return true;
        }

        explicit operator std::string() const
        {
            std::string sig;
            for (const auto& param : parameters)
            {
                sig+=fmt::format("{},", param->name);
            }
            if (not sig.empty())
            {
                sig.pop_back();
            }
            return fmt::format("({})", sig);
        }

        static std::string describe_args(const std::vector<std::shared_ptr<builder::value>>& args)
        {
            std::string sig;
            for (const auto& arg : args)
            {
                sig+=fmt::format("{},", arg->type->name);
            }
            if (not sig.empty())
            {
                sig.pop_back();
            }
            return fmt::format("({})", sig);
        }

        bool operator==(const signature& other) const
        {
            return this->type == other.type and this->parameters == other.parameters;
        }
    };
}

