//
// Created by dte on 12/15/2025.
//

#pragma once
#include <memory>
#include <vector>

#include "library.hpp"
#include "types/stack.hpp"

namespace zen::builder
{
    struct program
    {
        std::unordered_map<i64, std::shared_ptr<library>> libraries;
        std::unordered_map<i64, i64> links;
        std::vector<types::stack::i64> code;
        static std::shared_ptr<builder::program> create();
        void add(std::shared_ptr<builder::library>);
        void remove(const std::shared_ptr<builder::library>&);
        i64 link(const std::shared_ptr<builder::function>& fn);
        void run();
        private:
        std::shared_ptr<builder::function> get(i64 id) const;
    };
}

