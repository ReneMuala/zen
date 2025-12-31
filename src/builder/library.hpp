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
        std::list<std::shared_ptr<builder::function>> tests;
        void add_test(const std::shared_ptr<builder::function>&);
        i64 hash() const;
        std::unordered_map<i64,std::shared_ptr<builder::function>> functions;
        std::unordered_map<i64,std::shared_ptr<builder::type>> types;
        std::shared_ptr<function> get_function(i64);
        std::shared_ptr<type> get_type(i64);
        void add(std::shared_ptr<builder::function>);
        void add(std::shared_ptr<builder::type>);
        static std::shared_ptr<library> create(const std::string& name);
    };

}

