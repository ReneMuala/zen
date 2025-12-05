//
// Created by dte on 12/3/2025.
//
#pragma once

#include <memory>
#include <vector>

namespace zen::builder
{
    struct signature
    {
        std::shared_ptr<struct type> type;
        std::vector<std::shared_ptr<struct type>> parameters {};

        bool operator==(const signature& other) const
        {
            return this->type == other.type and this->parameters == other.parameters;
        }
    };
}

