//
// Created by dte on 12/15/2025.
//

#pragma once
#include <memory>

#include "function.hpp"

namespace zen::builder
{
    struct library
    {
        std::string name;
        int hash() const;
        std::unordered_map<i32,std::shared_ptr<builder::function>> functions;
        std::shared_ptr<function> get(i32);
        void add(const std::shared_ptr<builder::function> &);
        static std::shared_ptr<library> create(const std::string& name);
    };
}

