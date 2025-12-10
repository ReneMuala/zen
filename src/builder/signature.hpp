//
// Created by dte on 12/3/2025.
//
#pragma once

#include <memory>
#include <vector>
#include "type.hpp"
namespace zen::builder
{
    struct signature
    {
        std::shared_ptr<builder::type> type;
        std::vector<std::shared_ptr<builder::type>> parameters {};

        bool operator==(const signature& other) const
        {
            return this->type == other.type and this->parameters == other.parameters;
        }
    };
}

