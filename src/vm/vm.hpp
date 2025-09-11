//
// Created by dte on 3/21/2025.
//

#pragma once
#include <vector>
#include <cstdint>
// #include <ffi.h>
#include <map>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <fmt/base.h>

#define DECL_KAIZEN_ARITHMETICS_FOR_INTEGER_TYPE(T) \
add_##T, \
sub_##T,\
mul_##T,\
div_##T,\
mod_##T

#define DECL_KAIZEN_ARITHMETICS_FOR_FLOAT_TYPE(T) \
add_##T, \
sub_##T,\
mul_##T,\
div_##T

#define DECL_KAIZEN_RELATIONAL_FOR_TYPE(T) \
gt_##T,\
lt_##T,\
gte_##T,\
lte_##T,\
eq_##T,\
neq_##T

#define DECL_KAIZEN_CONVERSION_FOR_TYPE(T) \
T##_to_i64,\
T##_to_f64,\
T##_to_i8,\
T##_to_i32,\
T##_to_i16,\
T##_to_f32,\
T##_to_boolean

#define DECL_KAIZEN_PUSH_FOR_TYPE(T) \
push_ ## T

namespace zen
{
    using boolean = bool;
    using i8 = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;
    using f32 = float;
    using f64 = double;
    constexpr static auto ascii_art = R"(   .-') _   ('-.       .-') _
  (  OO) )_(  OO)     ( OO ) )
,(_)----.(,------.,--./ ,--,'
|       | |  .---'|   \ |  |\
'--.   /  |  |    |    \|  | )
(_/   /  (|  '--. |  .     |/
 /   /___ |  .--' |  |\    |
|        ||  `---.|  | \   |
`--------'`------'`--'  `--'  )";
    enum instruction
    {
        DECL_KAIZEN_PUSH_FOR_TYPE(i8),
        DECL_KAIZEN_PUSH_FOR_TYPE(i16),
        DECL_KAIZEN_PUSH_FOR_TYPE(i32),
        DECL_KAIZEN_PUSH_FOR_TYPE(i64),
        DECL_KAIZEN_PUSH_FOR_TYPE(f32),
        DECL_KAIZEN_PUSH_FOR_TYPE(f64),
        DECL_KAIZEN_PUSH_FOR_TYPE(boolean),
        // pop,
        DECL_KAIZEN_ARITHMETICS_FOR_INTEGER_TYPE(i8),
        DECL_KAIZEN_ARITHMETICS_FOR_INTEGER_TYPE(i16),
        DECL_KAIZEN_ARITHMETICS_FOR_INTEGER_TYPE(i32),
        DECL_KAIZEN_ARITHMETICS_FOR_INTEGER_TYPE(i64),
        DECL_KAIZEN_ARITHMETICS_FOR_FLOAT_TYPE(f32),
        DECL_KAIZEN_ARITHMETICS_FOR_FLOAT_TYPE(f64),

        DECL_KAIZEN_RELATIONAL_FOR_TYPE(i8),
        DECL_KAIZEN_RELATIONAL_FOR_TYPE(i16),
        DECL_KAIZEN_RELATIONAL_FOR_TYPE(i32),
        DECL_KAIZEN_RELATIONAL_FOR_TYPE(i64),
        DECL_KAIZEN_RELATIONAL_FOR_TYPE(f32),
        DECL_KAIZEN_RELATIONAL_FOR_TYPE(f64),

        DECL_KAIZEN_CONVERSION_FOR_TYPE(i8),
        DECL_KAIZEN_CONVERSION_FOR_TYPE(i16),
        DECL_KAIZEN_CONVERSION_FOR_TYPE(i32),
        DECL_KAIZEN_CONVERSION_FOR_TYPE(i64),
        DECL_KAIZEN_CONVERSION_FOR_TYPE(f32),
        DECL_KAIZEN_CONVERSION_FOR_TYPE(f64),

        boolean_and,
        boolean_or,
        boolean_not,

        // etc
        hlt,
        ret,
        // modify stack
        most,
        go,
        go_if,
        jump,
        jump_if,
        call,
        // ffi_call,
        // fetch_i8,
        // fetch_i64,
        // fetch_f64,
        // fetch_boolean,
        // send_i8,
        // send_i64,
        // send_f64,
        // send_boolean,
        // memory_of,
        placeholder,
    };
    class vm
    {
    public:


        struct stack
        {
            void * data = nullptr;
            i64 negative_stack_size = 0;
            void * operator-(const i64 & size);
            bool operator-=(const i64 & size);
            bool operator+=(const i64 & size);
            ~stack();
            [[nodiscard]] bool empty() const;
            [[nodiscard]] i64 size() const;
        };

    private:
        std::vector<i64> code;
        // std::map<i64, ffi_cif> callables;

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
        // bool register_ffi_callable(i64 function_pointer,std::vector<i64> args, i64 return_type)
        // {
        //     // ffi_cif & cif = callables[function_pointer];
        //     // return ffi_prep_cif(&cif, FFI_DEFAULT_ABI, args.size(), reinterpret_cast<ffi_type*>(return_type), reinterpret_cast<ffi_type**>(args.data())) == FFI_OK;
        // }
        static constexpr i64 ref(auto & some)
        {
           return reinterpret_cast<i64>(&some);
        }
        void load(std::vector<i64>& code);
        void run(const i64 & entry_point = 0);
        void run(stack & stack, const i64 & entry_point = 0);
    };
}
