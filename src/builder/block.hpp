//
// Created by dte on 9/22/2025.
//

#pragma once
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <fmt/base.h>
#include "scope.hpp"
#include "value.hpp"
#include "types/stack.hpp"

namespace zen::builder
{
    struct block : public scope, public std::enable_shared_from_this<block>
    {
        std::unordered_map<std::string, std::deque<std::shared_ptr<value>>> locals = {};
        types::stack::i64 stack_usage = {};
        bool did_pop = false;
        enum return_status
        {
            no_return = 0,
            branched_return = 1,
            concise_return = 1 << 3,
        } return_status = {no_return};

        std::shared_ptr<value> get_local(const std::string& name);

        void use_stack(const types::stack::i64 size);

        types::stack::i64 get_stack_usage() const;

        types::stack::i64 get_deep_stack_usage() const;

        void set_local(const std::string& name, const std::shared_ptr<builder::type>& t,
                       const types::stack::i64& stack_usage);

        std::shared_ptr<block> nested_scope = {};

        virtual void __dncd__push(std::shared_ptr<block> scope);

        decltype(locals)& ___dncd__deepest_locals();

        enum _pop_state
        {
            searching,
            found,
            popped,
        };
        /// do not call directly
        types::stack::i64 __dncd__pop(enum return_status& root_status);

        // improve this
        types::stack::i64 __dncd__pop(enum return_status& root_status, _pop_state& state);

        void update_return_status(enum return_status& root_status) const;

        /// do not call directly
        types::stack::i64 __dncd__peek(enum return_status& root_status);

        enum return_status get_return_status() const;

        void set_return_status(enum return_status status);

        void print_stack_tree() const;

        types::stack::i64 depth() const;

        static std::shared_ptr<block> create(const enum type type);

        [[nodiscard]] bool is(const enum type& t) const;

        bool in_loop() const;
    };
}
