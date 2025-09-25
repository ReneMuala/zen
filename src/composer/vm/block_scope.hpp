//
// Created by dte on 9/22/2025.
//

#pragma once
#include "scope.hpp"

namespace zen::composer::vm
{
    struct block_scope : public scope
    {
        enum return_status
        {
            no_return = 0,
            branched_return = 1,
            concise_return = 1 << 3,
        } return_status = {no_return};

        block_scope* nested_scope = {};

        virtual void push(block_scope* scope)
        {
            if (nested_scope)
                nested_scope->push(scope);
            else
            {
                nested_scope = scope;
            }
        }

        virtual void pop()
        {
            if (nested_scope)
            {
                if (nested_scope->nested_scope)
                    return nested_scope->pop();
                if (nested_scope->type == in_if and nested_scope->return_status == concise_return)
                    return_status = branched_return;
                else if (nested_scope->type == in_else_if and nested_scope->return_status == concise_return)
                    return_status = branched_return;
                else if (nested_scope->type == in_else and nested_scope->return_status == concise_return and (return_status == branched_return or return_status == concise_return))
                    return_status = concise_return;
                else
                    return_status = no_return;
                delete nested_scope;
                nested_scope = nullptr;
            }
        }

        enum return_status get_return_status() const
        {
            return nested_scope ? nested_scope->get_return_status() : return_status;
        }

        void set_return_status(enum return_status status)
        {
            if (nested_scope)
                nested_scope->set_return_status(status);
            else
                return_status = status;
        }

        [[nodiscard]] bool is(const enum type& t) const override
        {
            return t & type::in_block;
        }

        static block_scope* __unsafely_make_if()
        {
            const auto it = new block_scope;
            it->type = type::in_if;
            return it;
        }

        static block_scope* __unsafely_make_else()
        {
            const auto it = new block_scope;
            it->type = type::in_else;
            return it;
        }

        static block_scope* __unsafely_make_while_prologue()
        {
            const auto it = new block_scope;
            it->type = type::in_while_prologue;
            return it;
        }

        static block_scope* __unsafely_make_while_body()
        {
            const auto it = new block_scope;
            it->type = type::in_while_body;
            return it;
        }
    };
}

