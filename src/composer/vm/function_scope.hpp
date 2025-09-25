//
// Created by dte on 9/22/2025.
//

#pragma once
#include <optional>

#include "scope.hpp"
#include "block_scope.hpp"
#include "label.hpp"
#include "composer/composer.hpp"
namespace zen::composer::vm
{
    struct function_scope : public block_scope
    {
        struct return_data
        {
            std::optional<value> value = std::nullopt;
            std::optional<std::string> name = std::nullopt;
        } return_data = {};
        std::stack<label> labels = {};
        std::string name = {};
        i64 stack_usage = {};
        std::map<std::string, symbol> locals = {};

        [[nodiscard]] bool is(const enum type& t) const override
        {
            return t & type::in_function;
        }



        ~function_scope() override = default;
    };
}
