//
// Created by dte on 9/22/2025.
//

#pragma once
#include <optional>

#include "scope.hpp"
#include "block_scope.hpp"
#include "label.hpp"
#include "composer/composer.hpp"
#include "function.hpp"

namespace zen::composer::vm
{
    struct function_scope : public block_scope
    {
        struct return_data
        {
            std::shared_ptr<struct value> value = {};
            std::optional<std::string> name = std::nullopt;
        } return_data = {};
        std::stack<label> labels = {};
        std::string name = {};
        std::reference_wrapper<function> definition;
        explicit function_scope(const std::reference_wrapper<function> definition): definition(definition) {}
        ~function_scope() override = default;
    };
}
