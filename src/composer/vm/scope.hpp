//
// Created by dte on 9/22/2025.
//

#pragma once

namespace zen::composer::vm
{
    struct scope
    {
        virtual ~scope() = default;

        enum type {
            no_scope = 0,
            in_class = 1,
            in_block = 1 << 1,
            in_function = in_block | 1 << 2,
            in_if = in_function | 1 << 3,
            in_else = in_if | 1 << 4,
            in_else_if = in_else | 1 << 5,
            in_while_prologue = in_function | 1 << 6,
            in_while_body = in_function | 1 << 6,
        } type = {no_scope};

        [[nodiscard]] virtual bool is(const enum type &t) const
        {
            return t == type::no_scope;
        }
    };
}
