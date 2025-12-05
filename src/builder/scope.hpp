//
// Created by dte on 9/22/2025.
//

#pragma once
namespace zen::builder
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
            in_between_branches = in_else_if | 1 << 6,
            in_while_prologue = in_function | 1 << 7,
            in_while_body = in_function | 1 << 8,
            in_for = in_function | 1 << 9,
            in_for_body = in_for | 1 << 10,
        } type = {no_scope};

        [[nodiscard]] virtual bool is(const enum type &t) const
        {
            return (type & t)  == t;
        }
    };
}
