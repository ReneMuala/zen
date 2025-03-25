//
// Created by dte on 3/21/2025.
//

#pragma once
#include <vector>
#include <cstdint>
#include <stack>
#include <stdlib.h>
#include <fmt/base.h>

namespace zen
{
    enum instruction
    {
        push,
        pop,

        // arith for i64
        add_i64,
        sub_i64,
        mul_i64,
        div_i64,
        mod_i64,

        // relational for i64
        gt_i64,
        lt_i64,
        gte_i64,
        lte_i64,
        eq_i64,
        neq_i64,

        // arith for f64
        add_f64,
        sub_f64,
        mul_f64,
        div_f64,

        // relational for f64
        gt_f64,
        lt_f64,
        gte_f64,
        lte_f64,
        eq_f64,
        neq_f64,

        // logic for boolean
        boolean_and,
        boolean_or,

        // conversions & copies
        i64_to_f64,
        i64_to_i64,
        f64_to_i64,
        f64_to_f64,
        boolean_to_i64,
        i64_to_boolean,
        boolean_to_f64,
        f64_to_boolean,

        // unary for i64
        inc_i64,
        dec_i64,

        // unary for f64
        inc_f64,
        dec_f64,

        bit_and,
        bit_or,
        bit_xor,
        bit_not,
        // unary for boolean
        boolean_not,

        // etc
        hlt,
        // modify stack
        most,
        jump,
        jump_if,
    };
    class vm
    {
    public:
        using boolean = bool;
        using i64 = int64_t;
        using f64 = double;

        struct stack
        {
            void * data = nullptr;
            i64 negative_stack_size = 0;
            void * operator-(const i64 & size);
            bool operator-=(const i64 & size);
            bool operator+=(const i64 & size);
            ~stack();
            bool empty() const;
            i64 size() const;
        };

    private:
        std::vector<i64> code;

        template<typename type>
        static constexpr  void fetch_lhs_rhs(std::stack<type>& stack, type& lhs, type& rhs)
        {
            rhs = stack.top();
            stack.pop();
            lhs = stack.top();
            stack.pop();
        }

        template<typename type>
        static constexpr bool check_stack_for_pop(std::stack<type>& stack){
            if (stack.empty())
            {
                fmt::println(stderr, "runtime error: pop on an empty stack");
                return false;
            }
            return true;
        }

        template<typename type>
        static inline type* address(i64 address, vm::stack& stack)
        {
            return address > 0 ? reinterpret_cast<type*>(address) : static_cast<type*>(stack - -address);
        }

    public:
        static constexpr i64 ref(auto & some)
        {
           return reinterpret_cast<vm::i64>(&some);
        }
        void load(std::vector<i64>& code);
        void run(const i64 & entry_point = 0);
        void run(stack & stack, const i64 & entry_point = 0);
    };
}
