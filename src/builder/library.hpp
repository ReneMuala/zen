//
// Created by dte on 12/15/2025.
//

#pragma once
#include <memory>

#include "function.hpp"
#include "generic_context.hpp"

namespace zen::builder
{
    struct library
    {
        std::string name;
        std::list<std::shared_ptr<builder::function>> tests;
        void add_test(const std::shared_ptr<builder::function>&);
        i64 hash() const;
        std::expected<std::shared_ptr<type>, std::string> find_type_or_throw(const std::string& string);
        std::unordered_map<i64,std::shared_ptr<builder::function>> functions;
        std::unordered_map<i64,std::shared_ptr<builder::type>> types;
        std::unordered_map<i64,std::shared_ptr<builder::generic_context>> generic_types, generic_functions;
        std::shared_ptr<function> get_function(i64);
        std::shared_ptr<generic_context> get_generic_function(i64 id);
        std::shared_ptr<generic_context> get_generic_type(i64 id);
        std::shared_ptr<type> get_type(i64);
        void add(const std::shared_ptr<function>&);
        void add_generic_type(const std::shared_ptr<generic_context>& gc);
        void add_generic_function(const std::shared_ptr<generic_context>& gc);
        std::expected<bool, std::string> add(const std::shared_ptr<type>&);
        static std::shared_ptr<library> create(const std::string& name);
    };

}

