//
// Created by dte on 12/15/2025.
//

#ifndef ZEN_PROGRAM_HPP
#define ZEN_PROGRAM_HPP
#include <memory>
#include <vector>

#include "library.hpp"
#include "types/stack.hpp"

namespace zen::builder
{
    struct program
    {
        std::unordered_map<i32, std::shared_ptr<library>> libraries;
        std::unordered_map<i32, i64> links;
        std::vector<types::stack::i64> code;
        static std::shared_ptr<builder::program> create();
        void add(const std::shared_ptr<builder::library>&);
        void remove(const std::shared_ptr<builder::library>&);
        void link(const std::shared_ptr<builder::function>& fn);
        void run();
        private:
        std::shared_ptr<builder::function> get(i32 id) const;
    };
}

#endif //ZEN_PROGRAM_HPP
